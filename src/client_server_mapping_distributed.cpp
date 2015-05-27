/*!
   \file client_server_mapping.hpp
   \author Ha NGUYEN
   \since 27 Feb 2015
   \date 09 Mars 2015

   \brief Mapping between index client and server.
   Clients pre-calculate all information of server distribution.
 */
#include "client_server_mapping_distributed.hpp"
#include <limits>
#include <boost/functional/hash.hpp>
#include "utils.hpp"

namespace xios
{

CClientServerMappingDistributed::CClientServerMappingDistributed(const boost::unordered_map<size_t,int>& globalIndexOfServer,
                                                                 const MPI_Comm& clientIntraComm, bool isDataDistributed)
  : CClientServerMapping(), indexClientHash_(), countIndexGlobal_(0), countIndexServer_(0),
    indexGlobalBuffBegin_(), indexServerBuffBegin_(), requestRecvIndexServer_(), isDataDistributed_(isDataDistributed)
{
  clientIntraComm_ = clientIntraComm;
  MPI_Comm_size(clientIntraComm,&(nbClient_));
  MPI_Comm_rank(clientIntraComm,&clientRank_);
  computeHashIndex();
  computeDistributedServerIndex(globalIndexOfServer, clientIntraComm);
}

CClientServerMappingDistributed::~CClientServerMappingDistributed()
{
}

/*!
   Compute mapping global index of server which client sends to.
   \param [in] globalIndexOnClient global index client has
   \param [in] localIndexOnClient local index on client
*/
//void CClientServerMappingDistributed::computeServerIndexMapping(const CArray<size_t,1>& globalIndexOnClient,
//                                                                const CArray<int,1>& localIndexOnClient)
void CClientServerMappingDistributed::computeServerIndexMapping(const CArray<size_t,1>& globalIndexOnClient)
{
  size_t ssize = globalIndexOnClient.numElements(), hashedIndex;

  std::vector<size_t>::const_iterator itbClientHash = indexClientHash_.begin(), itClientHash,
                                      iteClientHash = indexClientHash_.end();
  std::map<int, std::vector<size_t> > client2ClientIndexGlobal;
  std::map<int, std::vector<int> > client2ClientIndexServer;

  // Number of global index whose mapping server can be found out thanks to index-server mapping
  int nbIndexAlreadyOnClient = 0;

  // Number of global index whose mapping server are on other clients
  int nbIndexSendToOthers = 0;
  HashXIOS<size_t> hashGlobalIndex;
  for (int i = 0; i < ssize; ++i)
  {
    size_t globalIndexClient = globalIndexOnClient(i);
    hashedIndex  = hashGlobalIndex(globalIndexClient);
    itClientHash = std::upper_bound(itbClientHash, iteClientHash, hashedIndex);
    if (iteClientHash != itClientHash)
    {
      int indexClient = std::distance(itbClientHash, itClientHash)-1;

      if (clientRank_ == indexClient)
      {
        (indexGlobalOnServer_[globalIndexToServerMapping_[globalIndexClient]]).push_back(globalIndexClient);
        ++nbIndexAlreadyOnClient;
      }
      else
      {
        client2ClientIndexGlobal[indexClient].push_back(globalIndexClient);
        ++nbIndexSendToOthers;
      }
    }
  }

  int* sendBuff = new int[nbClient_];
  for (int i = 0; i < nbClient_; ++i) sendBuff[i] = 0;
  std::map<int, std::vector<size_t> >::iterator it  = client2ClientIndexGlobal.begin(),
                                                ite = client2ClientIndexGlobal.end();
  for (; it != ite; ++it) sendBuff[it->first] = 1;
  int* recvBuff = new int[nbClient_];
  MPI_Allreduce(sendBuff, recvBuff, nbClient_, MPI_INT, MPI_SUM, clientIntraComm_);

  std::list<MPI_Request> sendRequest;
  if (0 != nbIndexSendToOthers)
      for (it = client2ClientIndexGlobal.begin(); it != ite; ++it)
         sendIndexGlobalToClients(it->first, it->second, clientIntraComm_, sendRequest);

  int nbDemandingClient = recvBuff[clientRank_], nbIndexServerReceived = 0;
  // Receiving demand as well as the responds from other clients
  // The demand message contains global index; meanwhile the responds have server index information
  // Buffer to receive demand from other clients, it can be allocated or not depending whether it has demand(s)
  unsigned long* recvBuffIndexGlobal = 0;
  int maxNbIndexDemandedFromOthers = (nbIndexAlreadyOnClient >= globalIndexToServerMapping_.size())
                                   ? 0 : (globalIndexToServerMapping_.size() - nbIndexAlreadyOnClient);
  if (!isDataDistributed_) maxNbIndexDemandedFromOthers = nbDemandingClient * globalIndexToServerMapping_.size(); // Not very optimal but it's general

  if (0 != maxNbIndexDemandedFromOthers)
    recvBuffIndexGlobal = new unsigned long[maxNbIndexDemandedFromOthers];

  // Buffer to receive respond from other clients, it can be allocated or not depending whether it demands other clients
  int* recvBuffIndexServer = 0;
  int nbIndexReceivedFromOthers = nbIndexSendToOthers;
//  int nbIndexReceivedFromOthers = globalIndexToServerMapping_.size() - nbIndexAlreadyOnClient;
  if (0 != nbIndexReceivedFromOthers)
    recvBuffIndexServer = new int[nbIndexReceivedFromOthers];

  std::map<int, MPI_Request>::iterator itRequest;
  std::vector<int> demandAlreadyReceived, repondAlreadyReceived;


  resetReceivingRequestAndCount();
  while ((0 < nbDemandingClient) || (!sendRequest.empty()) ||
         (nbIndexServerReceived < nbIndexReceivedFromOthers))
  {
    // Just check whether a client has any demand from other clients.
    // If it has, then it should send responds to these client(s)
    probeIndexGlobalMessageFromClients(recvBuffIndexGlobal, maxNbIndexDemandedFromOthers);
    if (0 < nbDemandingClient)
    {
      for (itRequest = requestRecvIndexGlobal_.begin();
           itRequest != requestRecvIndexGlobal_.end(); ++itRequest)
      {
        int flagIndexGlobal, count;
        MPI_Status statusIndexGlobal;

        MPI_Test(&(itRequest->second), &flagIndexGlobal, &statusIndexGlobal);
        if (true == flagIndexGlobal)
        {
          MPI_Get_count(&statusIndexGlobal, MPI_UNSIGNED_LONG, &count);
          int clientSourceRank = statusIndexGlobal.MPI_SOURCE;
          unsigned long* beginBuff = indexGlobalBuffBegin_[clientSourceRank];
          for (int i = 0; i < count; ++i)
          {
            client2ClientIndexServer[clientSourceRank].push_back(globalIndexToServerMapping_[*(beginBuff+i)]);
          }
          sendIndexServerToClients(clientSourceRank, client2ClientIndexServer[clientSourceRank], clientIntraComm_, sendRequest);
          --nbDemandingClient;

          demandAlreadyReceived.push_back(clientSourceRank);
        }
      }
      for (int i = 0; i< demandAlreadyReceived.size(); ++i)
        requestRecvIndexGlobal_.erase(demandAlreadyReceived[i]);
    }

    testSendRequest(sendRequest);

    // In some cases, a client need to listen respond from other clients about server information
    // Ok, with the information, a client can fill in its server-global index map.
    probeIndexServerMessageFromClients(recvBuffIndexServer, nbIndexReceivedFromOthers);
    for (itRequest = requestRecvIndexServer_.begin();
         itRequest != requestRecvIndexServer_.end();
         ++itRequest)
    {
      int flagIndexServer, count;
      MPI_Status statusIndexServer;

      MPI_Test(&(itRequest->second), &flagIndexServer, &statusIndexServer);
      if (true == flagIndexServer)
      {
        MPI_Get_count(&statusIndexServer, MPI_INT, &count);
        int clientSourceRank = statusIndexServer.MPI_SOURCE;
        int* beginBuff = indexServerBuffBegin_[clientSourceRank];
        std::vector<size_t>& globalIndexTmp = client2ClientIndexGlobal[clientSourceRank];
        for (int i = 0; i < count; ++i)
        {
          (indexGlobalOnServer_[*(beginBuff+i)]).push_back(globalIndexTmp[i]);
        }
        nbIndexServerReceived += count;
        repondAlreadyReceived.push_back(clientSourceRank);
      }
    }

    for (int i = 0; i< repondAlreadyReceived.size(); ++i)
      requestRecvIndexServer_.erase(repondAlreadyReceived[i]);
    repondAlreadyReceived.resize(0);
  }

  if (0 != recvBuffIndexGlobal) delete recvBuffIndexGlobal;
  if (0 != recvBuffIndexServer) delete recvBuffIndexServer;
  delete [] sendBuff;
  delete [] recvBuff;
}

/*!
  Compute the hash index distribution of whole size_t space then each client will have a range of this distribution
*/
void CClientServerMappingDistributed::computeHashIndex()
{
  // Compute range of hash index for each client
  indexClientHash_.resize(nbClient_+1);
  size_t nbHashIndexMax = std::numeric_limits<size_t>::max();
  size_t nbHashIndex;
  indexClientHash_[0] = 0;
  for (int i = 1; i < nbClient_; ++i)
  {
    nbHashIndex = nbHashIndexMax / nbClient_;
    if (i < (nbHashIndexMax%nbClient_)) ++nbHashIndex;
    indexClientHash_[i] = indexClientHash_[i-1] + nbHashIndex;
  }
  indexClientHash_[nbClient_] = nbHashIndexMax;
}

/*!
  Compute distribution of global index for servers
  Each client already holds a piece of information about global index and the corresponding server.
This information is redistributed into size_t space in which each client possesses a specific range of index.
After the redistribution, each client as well as its range of index contains all necessary information about server.
  \param [in] globalIndexOfServer global index and the corresponding server
  \param [in] clientIntraComm client joining distribution process.
*/
void CClientServerMappingDistributed::computeDistributedServerIndex(const boost::unordered_map<size_t,int>& globalIndexOfServer,
                                                                    const MPI_Comm& clientIntraComm)
{
  int* sendBuff = new int[nbClient_];
  int* sendNbIndexBuff = new int[nbClient_];
  for (int i = 0; i < nbClient_; ++i)
  {
    sendBuff[i] = 0; sendNbIndexBuff[i] = 0;
  }

  // Compute size of sending and receving buffer
  std::map<int, std::vector<size_t> > client2ClientIndexGlobal;
  std::map<int, std::vector<int> > client2ClientIndexServer;

  std::vector<size_t>::const_iterator itbClientHash = indexClientHash_.begin(), itClientHash,
                                      iteClientHash = indexClientHash_.end();
  boost::unordered_map<size_t,int>::const_iterator it  = globalIndexOfServer.begin(),
                                                   ite = globalIndexOfServer.end();
  HashXIOS<size_t> hashGlobalIndex;
  for (; it != ite; ++it)
  {
    size_t hashIndex = hashGlobalIndex(it->first);
    itClientHash = std::upper_bound(itbClientHash, iteClientHash, hashIndex);
    if (itClientHash != iteClientHash)
    {
      int indexClient = std::distance(itbClientHash, itClientHash)-1;
      if (clientRank_ == indexClient)
      {
        globalIndexToServerMapping_.insert(std::make_pair<size_t,int>(it->first, it->second));
      }
      else
      {
        sendBuff[indexClient] = 1;
        ++sendNbIndexBuff[indexClient];
        client2ClientIndexGlobal[indexClient].push_back(it->first);
        client2ClientIndexServer[indexClient].push_back(it->second);
      }
    }
  }

  // Calculate from how many clients each client receive message.
  int* recvBuff = new int[nbClient_];
  MPI_Allreduce(sendBuff, recvBuff, nbClient_, MPI_INT, MPI_SUM, clientIntraComm);
  int recvNbClient = recvBuff[clientRank_];

  // Calculate size of buffer for receiving message
  int* recvNbIndexBuff = new int[nbClient_];
  MPI_Allreduce(sendNbIndexBuff, recvNbIndexBuff, nbClient_, MPI_INT, MPI_SUM, clientIntraComm);
  int recvNbIndexCount = recvNbIndexBuff[clientRank_];
  unsigned long* recvIndexGlobalBuff = new unsigned long[recvNbIndexCount];
  int* recvIndexServerBuff = new int[recvNbIndexCount];

  // If a client holds information about global index and servers which don't belong to it,
  // it will send a message to the correct clients.
  // Contents of the message are global index and its corresponding server index
  std::list<MPI_Request> sendRequest;
  std::map<int, std::vector<size_t> >::iterator itGlobal  = client2ClientIndexGlobal.begin(),
                                                iteGlobal = client2ClientIndexGlobal.end();
  for ( ;itGlobal != iteGlobal; ++itGlobal)
    sendIndexGlobalToClients(itGlobal->first, itGlobal->second, clientIntraComm, sendRequest);
  std::map<int, std::vector<int> >::iterator itServer  = client2ClientIndexServer.begin(),
                                             iteServer = client2ClientIndexServer.end();
  for (; itServer != iteServer; ++itServer)
    sendIndexServerToClients(itServer->first, itServer->second, clientIntraComm, sendRequest);

  std::map<int, MPI_Request>::iterator itRequestIndexGlobal, itRequestIndexServer;
  std::map<int, int> countBuffIndexServer, countBuffIndexGlobal;
  std::vector<int> processedList;

  bool isFinished = (0 == recvNbClient) ? true : false;

  // Just to make sure before listening message, all counting index and receiving request have already beeen reset
  resetReceivingRequestAndCount();

  // Now each client trys to listen to demand from others.
  // If they have message, it processes: pushing global index and corresponding server to its map
  while (!isFinished || (!sendRequest.empty()))
  {
    testSendRequest(sendRequest);
    probeIndexGlobalMessageFromClients(recvIndexGlobalBuff, recvNbIndexCount);

    // Processing complete request
    for (itRequestIndexGlobal = requestRecvIndexGlobal_.begin();
         itRequestIndexGlobal != requestRecvIndexGlobal_.end();
         ++itRequestIndexGlobal)
    {
      int rank = itRequestIndexGlobal->first;
      int countIndexGlobal = computeBuffCountIndexGlobal(itRequestIndexGlobal->second);
      if (0 != countIndexGlobal)
        countBuffIndexGlobal[rank] = countIndexGlobal;
    }

    probeIndexServerMessageFromClients(recvIndexServerBuff, recvNbIndexCount);
    for (itRequestIndexServer = requestRecvIndexServer_.begin();
         itRequestIndexServer != requestRecvIndexServer_.end();
         ++itRequestIndexServer)
    {
      int rank = itRequestIndexServer->first;
      int countIndexServer = computeBuffCountIndexServer(itRequestIndexServer->second);
      if (0 != countIndexServer)
        countBuffIndexServer[rank] = countIndexServer;
    }

    for (std::map<int, int>::iterator it = countBuffIndexGlobal.begin();
                                      it != countBuffIndexGlobal.end(); ++it)
    {
      int rank = it->first;
      if (countBuffIndexServer.end() != countBuffIndexServer.find(rank))
      {
        processReceivedRequest(indexGlobalBuffBegin_[rank], indexServerBuffBegin_[rank], it->second);
        processedList.push_back(rank);
        --recvNbClient;
      }
    }

    for (int i = 0; i < processedList.size(); ++i)
    {
      requestRecvIndexServer_.erase(processedList[i]);
      requestRecvIndexGlobal_.erase(processedList[i]);
      countBuffIndexGlobal.erase(processedList[i]);
      countBuffIndexServer.erase(processedList[i]);
    }

    if (0 == recvNbClient) isFinished = true;
  }

  delete [] sendBuff;
  delete [] sendNbIndexBuff;
  delete [] recvBuff;
  delete [] recvNbIndexBuff;
  delete [] recvIndexGlobalBuff;
  delete [] recvIndexServerBuff;
}

/*!
  Probe and receive message containg global index from other clients.
  Each client can send a message of global index to other clients to fulfill their maps.
Each client probes message from its queue then if the message is ready, it will be put into the receiving buffer
  \param [in] recvIndexGlobalBuff buffer dedicated for receiving global index
  \param [in] recvNbIndexCount size of the buffer
*/
void CClientServerMappingDistributed::probeIndexGlobalMessageFromClients(unsigned long* recvIndexGlobalBuff, int recvNbIndexCount)
{
  MPI_Status statusIndexGlobal;
  int flagIndexGlobal, count;

  // Probing for global index
  MPI_Iprobe(MPI_ANY_SOURCE, 15, clientIntraComm_, &flagIndexGlobal, &statusIndexGlobal);
  if ((true == flagIndexGlobal) && (countIndexGlobal_ < recvNbIndexCount))
  {
    MPI_Get_count(&statusIndexGlobal, MPI_UNSIGNED_LONG, &count);
    indexGlobalBuffBegin_.insert(std::make_pair<int, unsigned long*>(statusIndexGlobal.MPI_SOURCE, recvIndexGlobalBuff+countIndexGlobal_));
    MPI_Irecv(recvIndexGlobalBuff+countIndexGlobal_, count, MPI_UNSIGNED_LONG,
              statusIndexGlobal.MPI_SOURCE, 15, clientIntraComm_,
              &requestRecvIndexGlobal_[statusIndexGlobal.MPI_SOURCE]);
    countIndexGlobal_ += count;
  }
}

/*!
  Probe and receive message containg server index from other clients.
  Each client can send a message of server index to other clients to fulfill their maps.
Each client probes message from its queue then if the message is ready, it will be put into the receiving buffer
  \param [in] recvIndexServerBuff buffer dedicated for receiving server index
  \param [in] recvNbIndexCount size of the buffer
*/
void CClientServerMappingDistributed::probeIndexServerMessageFromClients(int* recvIndexServerBuff, int recvNbIndexCount)
{
  MPI_Status statusIndexServer;
  int flagIndexServer, count;

  // Probing for server index
  MPI_Iprobe(MPI_ANY_SOURCE, 12, clientIntraComm_, &flagIndexServer, &statusIndexServer);
  if ((true == flagIndexServer) && (countIndexServer_ < recvNbIndexCount))
  {
    MPI_Get_count(&statusIndexServer, MPI_INT, &count);
    indexServerBuffBegin_.insert(std::make_pair<int, int*>(statusIndexServer.MPI_SOURCE, recvIndexServerBuff+countIndexServer_));
    MPI_Irecv(recvIndexServerBuff+countIndexServer_, count, MPI_INT,
              statusIndexServer.MPI_SOURCE, 12, clientIntraComm_,
              &requestRecvIndexServer_[statusIndexServer.MPI_SOURCE]);

    countIndexServer_ += count;
  }
}

/*!
  Send message containing global index to clients
  \param [in] clientDestRank rank of destination client
  \param [in] indexGlobal global index to send
  \param [in] clientIntraComm communication group of client
  \param [in] requestSendIndexGlobal list of sending request
*/
void CClientServerMappingDistributed::sendIndexGlobalToClients(int clientDestRank, std::vector<size_t>& indexGlobal,
                                                               const MPI_Comm& clientIntraComm,
                                                               std::list<MPI_Request>& requestSendIndexGlobal)
{
  MPI_Request request;
  requestSendIndexGlobal.push_back(request);
  MPI_Isend(&(indexGlobal)[0], (indexGlobal).size(), MPI_UNSIGNED_LONG,
            clientDestRank, 15, clientIntraComm, &(requestSendIndexGlobal.back()));
}

/*!
  Send message containing server index to clients
  \param [in] clientDestRank rank of destination client
  \param [in] indexServer server index to send
  \param [in] clientIntraComm communication group of client
  \param [in] requestSendIndexServer list of sending request
*/
void CClientServerMappingDistributed::sendIndexServerToClients(int clientDestRank, std::vector<int>& indexServer,
                                                               const MPI_Comm& clientIntraComm,
                                                               std::list<MPI_Request>& requestSendIndexServer)
{
  MPI_Request request;
  requestSendIndexServer.push_back(request);
  MPI_Isend(&(indexServer)[0], (indexServer).size(), MPI_INT,
            clientDestRank, 12, clientIntraComm, &(requestSendIndexServer.back()));
}

/*!
  Verify status of sending request
  \param [in] sendRequest sending request to verify
*/
void CClientServerMappingDistributed::testSendRequest(std::list<MPI_Request>& sendRequest)
{
  int flag = 0;
  MPI_Status status;
  std::list<MPI_Request>::iterator itRequest;
  int sizeListRequest = sendRequest.size();
  int idx = 0;
  while (idx < sizeListRequest)
  {
    bool isErased = false;
    for (itRequest = sendRequest.begin(); itRequest != sendRequest.end(); ++itRequest)
    {
      MPI_Test(&(*itRequest), &flag, &status);
      if (true == flag)
      {
        isErased = true;
        break;
      }
    }
    if (true == isErased) sendRequest.erase(itRequest);
    ++idx;
  }
}

/*!
  Process the received request. Pushing global index and server index into map
  \param[in] buffIndexGlobal pointer to the begining of buffer containing global index
  \param[in] buffIndexServer pointer to the begining of buffer containing server index
  \param[in] count size of received message
*/
void CClientServerMappingDistributed::processReceivedRequest(unsigned long* buffIndexGlobal, int* buffIndexServer, int count)
{
  for (int i = 0; i < count; ++i)
    globalIndexToServerMapping_.insert(std::make_pair<size_t,int>(*(buffIndexGlobal+i),*(buffIndexServer+i)));
}

/*!
  Compute size of message containing global index
  \param[in] requestRecv request of message
*/
int CClientServerMappingDistributed::computeBuffCountIndexGlobal(MPI_Request& requestRecv)
{
  int flag, count = 0;
  MPI_Status status;

  MPI_Test(&requestRecv, &flag, &status);
  if (true == flag)
  {
    MPI_Get_count(&status, MPI_UNSIGNED_LONG, &count);
  }

  return count;
}

/*!
  Compute size of message containing server index
  \param[in] requestRecv request of message
*/
int CClientServerMappingDistributed::computeBuffCountIndexServer(MPI_Request& requestRecv)
{
  int flag, count = 0;
  MPI_Status status;

  MPI_Test(&requestRecv, &flag, &status);
  if (true == flag)
  {
    MPI_Get_count(&status, MPI_INT, &count);
  }

  return count;
}

/*!
  Reset all receiving request map and counter
*/
void CClientServerMappingDistributed::resetReceivingRequestAndCount()
{
  countIndexGlobal_ = countIndexServer_ = 0;
  requestRecvIndexGlobal_.clear();
  requestRecvIndexServer_.clear();
  indexGlobalBuffBegin_.clear();
  indexServerBuffBegin_.clear();
}

}