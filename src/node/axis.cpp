#include "axis.hpp"

#include "attribute_template.hpp"
#include "object_template.hpp"
#include "group_template.hpp"
#include "message.hpp"
#include "type.hpp"
#include "context.hpp"
#include "context_client.hpp"
#include "context_server.hpp"
#include "xios_spl.hpp"
#include "server_distribution_description.hpp"
#include "client_server_mapping_distributed.hpp"
#include "distribution_client.hpp"

namespace xios {

   /// ////////////////////// Definitions ////////////////////// ///

   CAxis::CAxis(void)
      : CObjectTemplate<CAxis>()
      , CAxisAttributes(), isChecked(false), relFiles(), areClientAttributesChecked_(false)
      , isClientAfterTransformationChecked(false)
      , hasBounds(false), isCompressible_(false)
      , numberWrittenIndexes_(), totalNumberWrittenIndexes_(), offsetWrittenIndexes_()
      , transformationMap_(), hasValue(false), hasLabel(false)
      , computedWrittenIndex_(false)
	  , clients()
   {
   }

   CAxis::CAxis(const StdString & id)
      : CObjectTemplate<CAxis>(id)
      , CAxisAttributes(), isChecked(false), relFiles(), areClientAttributesChecked_(false)
      , isClientAfterTransformationChecked(false)
      , hasBounds(false), isCompressible_(false)
      , numberWrittenIndexes_(), totalNumberWrittenIndexes_(), offsetWrittenIndexes_()
      , transformationMap_(), hasValue(false), hasLabel(false)
      , computedWrittenIndex_(false)
	  , clients()
   {
   }

   CAxis::~CAxis(void)
   { /* Ne rien faire de plus */ }

   std::map<StdString, ETranformationType> CAxis::transformationMapList_ = std::map<StdString, ETranformationType>();
   bool CAxis::dummyTransformationMapList_ = CAxis::initializeTransformationMap(CAxis::transformationMapList_);
   bool CAxis::initializeTransformationMap(std::map<StdString, ETranformationType>& m)
   TRY
   {
     m["zoom_axis"] = TRANS_ZOOM_AXIS;
     m["interpolate_axis"] = TRANS_INTERPOLATE_AXIS;
     m["extract_axis"] = TRANS_EXTRACT_AXIS;
     m["inverse_axis"] = TRANS_INVERSE_AXIS;
     m["reduce_domain"] = TRANS_REDUCE_DOMAIN_TO_AXIS;
     m["reduce_axis"] = TRANS_REDUCE_AXIS_TO_AXIS;
     m["extract_domain"] = TRANS_EXTRACT_DOMAIN_TO_AXIS;
     m["temporal_splitting"] = TRANS_TEMPORAL_SPLITTING;
     m["duplicate_scalar"] = TRANS_DUPLICATE_SCALAR_TO_AXIS;

   }
   CATCH

   ///---------------------------------------------------------------

   const std::set<StdString> & CAxis::getRelFiles(void) const
   TRY
   {
      return (this->relFiles);
   }
   CATCH

   bool CAxis::IsWritten(const StdString & filename) const
   TRY
   {
      return (this->relFiles.find(filename) != this->relFiles.end());
   }
   CATCH

   bool CAxis::isWrittenCompressed(const StdString& filename) const
   TRY
   {
      return (this->relFilesCompressed.find(filename) != this->relFilesCompressed.end());
   }
   CATCH

   bool CAxis::isDistributed(void) const
   TRY
   {
      bool distributed = (!this->begin.isEmpty() && !this->n.isEmpty() && (this->begin + this->n < this->n_glo)) ||
             (!this->n.isEmpty() && (this->n != this->n_glo));
      // A condition to make sure that if there is only one client, axis
      // should be considered to be distributed. This should be a temporary solution     
      distributed |= (1 == CContext::getCurrent()->intraCommSize_);
      return distributed;
   }
   CATCH

   /*!
    * Compute if the axis can be ouput in a compressed way.
    * In this case the workflow view on server side must be the same
    * than the full view for all context rank. The result is stored on
    * internal isCompressible_ attribute.
    */
   void CAxis::computeIsCompressible(void)
   TRY
   {
     // mesh is compressible contains some masked or indexed value, ie if full view is different of workflow view.
     // But now assume that the size of the 2 view must be equal for everybody. True on server side
     int isSameView = getLocalView(CElementView::FULL)->getSize() ==  getLocalView(CElementView::WORKFLOW)->getSize();
     MPI_Allreduce(MPI_IN_PLACE, &isSameView, 1, MPI_INT, MPI_LAND, CContext::getCurrent()->getIntraComm()) ;
     if (isSameView) isCompressible_ = false ;
     else isCompressible_ = true ;
     isCompressibleComputed_=true ;
   }
   CATCH

   void CAxis::addRelFile(const StdString & filename)
   TRY
   {
      this->relFiles.insert(filename);
   }
   CATCH_DUMP_ATTR

   void CAxis::addRelFileCompressed(const StdString& filename)
   TRY
   {
      this->relFilesCompressed.insert(filename);
   }
   CATCH_DUMP_ATTR

   //----------------------------------------------------------------

   /*!
     Returns the number of indexes written by each server.
     \return the number of indexes written by each server
   */
   int CAxis::getNumberWrittenIndexes(MPI_Comm writtenCom)
   TRY
   {
     int writtenSize;
     MPI_Comm_size(writtenCom, &writtenSize);
     return numberWrittenIndexes_[writtenSize];
   }
   CATCH_DUMP_ATTR

   /*!
     Returns the total number of indexes written by the servers.
     \return the total number of indexes written by the servers
   */
   int CAxis::getTotalNumberWrittenIndexes(MPI_Comm writtenCom)
   TRY
   {
     int writtenSize;
     MPI_Comm_size(writtenCom, &writtenSize);
     return totalNumberWrittenIndexes_[writtenSize];
   }
   CATCH_DUMP_ATTR

   /*!
     Returns the offset of indexes written by each server.
     \return the offset of indexes written by each server
   */
   int CAxis::getOffsetWrittenIndexes(MPI_Comm writtenCom)
   TRY
   {
     int writtenSize;
     MPI_Comm_size(writtenCom, &writtenSize);
     return offsetWrittenIndexes_[writtenSize];
   }
   CATCH_DUMP_ATTR

   CArray<int, 1>& CAxis::getCompressedIndexToWriteOnServer(MPI_Comm writtenCom)
   TRY
   {
     int writtenSize;
     MPI_Comm_size(writtenCom, &writtenSize);
     return compressedIndexToWriteOnServer[writtenSize];
   }
   CATCH_DUMP_ATTR

   //----------------------------------------------------------------

   /*!
    * Compute the minimum buffer size required to send the attributes to the server(s).
    *
    * \return A map associating the server rank with its minimum buffer size.
    */
   std::map<int, StdSize> CAxis::getAttributesBufferSize(CContextClient* client, const std::vector<int>& globalDim, int orderPositionInGrid,
                                                         CServerDistributionDescription::ServerDistributionType distType)
   TRY
   {

     std::map<int, StdSize> attributesSizes = getMinimumBufferSizeForAttributes(client);

//     bool isNonDistributed = (n_glo == n);
     bool isDistributed = (orderPositionInGrid == CServerDistributionDescription::defaultDistributedDimension(globalDim.size(), distType))
    		                 || (index.numElements() != n_glo);

     if (client->isServerLeader())
     {
       // size estimation for sendServerAttribut
       size_t size = 6 * sizeof(size_t);
       // size estimation for sendNonDistributedValue
       if (!isDistributed)
       {
//         size = std::max(size, CArray<double,1>::size(n_glo) + (isCompressible_ ? CArray<int,1>::size(n_glo) : 0));
         size += CArray<int,1>::size(n_glo);
         size += CArray<int,1>::size(n_glo);
         size += CArray<bool,1>::size(n_glo);
         size += CArray<double,1>::size(n_glo);
         if (hasBounds)
           size += CArray<double,2>::size(2*n_glo);
         if (hasLabel)
          size += CArray<StdString,1>::size(n_glo);
       }
       size += CEventClient::headerSize + getId().size() + sizeof(size_t);

       const std::list<int>& ranks = client->getRanksServerLeader();
       for (std::list<int>::const_iterator itRank = ranks.begin(), itRankEnd = ranks.end(); itRank != itRankEnd; ++itRank)
       {
         if (size > attributesSizes[*itRank])
           attributesSizes[*itRank] = size;
       }
       const std::list<int>& ranksNonLeaders = client->getRanksServerNotLeader();
       for (std::list<int>::const_iterator itRank = ranksNonLeaders.begin(), itRankEnd = ranksNonLeaders.end(); itRank != itRankEnd; ++itRank)
       {
         if (size > attributesSizes[*itRank])
           attributesSizes[*itRank] = size;
       }

     }

     if (isDistributed)
     {
       // size estimation for sendDistributedValue
       std::unordered_map<int, vector<size_t> >::const_iterator it, ite = indSrv_[client->serverSize].end();
       for (it = indSrv_[client->serverSize].begin(); it != ite; ++it)
       {
         size_t size = 6 * sizeof(size_t);
         size += CArray<int,1>::size(it->second.size());
         size += CArray<int,1>::size(it->second.size());
         size += CArray<bool,1>::size(it->second.size());
         size += CArray<double,1>::size(it->second.size());
         if (hasBounds)
           size += CArray<double,2>::size(2 * it->second.size());
         if (hasLabel)
           size += CArray<StdString,1>::size(it->second.size());

         size += CEventClient::headerSize + getId().size() + sizeof(size_t);
         if (size > attributesSizes[it->first])
           attributesSizes[it->first] = size;
       }
     }
     return attributesSizes;
   }
   CATCH_DUMP_ATTR

   //----------------------------------------------------------------

   StdString CAxis::GetName(void)   { return (StdString("axis")); }
   StdString CAxis::GetDefName(void){ return (CAxis::GetName()); }
   ENodeType CAxis::GetType(void)   { return (eAxis); }

   //----------------------------------------------------------------

   CAxis* CAxis::createAxis()
   TRY
   {
     CAxis* axis = CAxisGroup::get("axis_definition")->createChild();
     return axis;
   }
   CATCH

   /*!
     Check common attributes of an axis.
     This check should be done in the very beginning of work flow
   */
   void CAxis::checkAttributes(void)
   TRY
   {
     if (checkAttributes_done_) return ;

     CContext* context=CContext::getCurrent();

     if (this->n_glo.isEmpty())
        ERROR("CAxis::checkAttributes(void)",
              << "[ id = '" << getId() << "' , context = '" << CObjectFactory::GetCurrentContextId() << "' ] "
              << "The axis is wrongly defined, attribute 'n_glo' must be specified");
      StdSize size = this->n_glo.getValue();

      if (!this->index.isEmpty())
      {
        if (n.isEmpty()) n = index.numElements();

        // It's not so correct but if begin is not the first value of index 
        // then data on the local axis has user-defined distribution. In this case, begin has no meaning.
        if (begin.isEmpty()) begin = index(0);         
      }
      else 
      {
        if (!this->begin.isEmpty())
        {
          if (begin < 0 || begin > size - 1)
            ERROR("CAxis::checkAttributes(void)",
                  << "[ id = '" << getId() << "' , context = '" << CObjectFactory::GetCurrentContextId() << "' ] "
                  << "The axis is wrongly defined, attribute 'begin' (" << begin.getValue() << ") must be non-negative and smaller than size-1 (" << size - 1 << ").");
        }
        else this->begin.setValue(0);

        if (!this->n.isEmpty())
        {
          if (n < 0 || n > size)
            ERROR("CAxis::checkAttributes(void)",
                  << "[ id = '" << getId() << "' , context = '" << CObjectFactory::GetCurrentContextId() << "' ] "
                  << "The axis is wrongly defined, attribute 'n' (" << n.getValue() << ") must be non-negative and smaller than size (" << size << ").");
        }
        else this->n.setValue(size);

        {
          index.resize(n);
          for (int i = 0; i < n; ++i) index(i) = i+begin;
        }
      }

      if (!this->value.isEmpty())
      {
        StdSize true_size = value.numElements();
        if (this->n.getValue() != true_size)
          ERROR("CAxis::checkAttributes(void)",
              << "[ id = '" << getId() << "' , context = '" << CObjectFactory::GetCurrentContextId() << "' ] "
              << "The axis is wrongly defined, attribute 'value' has a different size (" << true_size
              << ") than the one defined by the \'size\' attribute (" << n.getValue() << ").");
        this->hasValue = true;
      }

      this->checkBounds();
      this->checkMask();
      this->checkData();
      this->checkLabel();
      initializeLocalElement() ;
      addFullView() ;
      addWorkflowView() ;
      addModelView() ;

      checkAttributes_done_ = true ;
   }
   CATCH_DUMP_ATTR



   /*!
      Check the validity of data, fill in values if any, and apply mask.
   */
   void CAxis::checkData()
   TRY
   {
      if (data_begin.isEmpty()) data_begin.setValue(0);

      if (data_n.isEmpty())
      {
        data_n.setValue(n);
      }
      else if (data_n.getValue() < 0)
      {
        ERROR("CAxis::checkData(void)",
              << "[ id = " << this->getId() << " , context = '" << CObjectFactory::GetCurrentContextId() << " ] "
              << "The data size should be strictly positive ('data_n' = " << data_n.getValue() << ").");
      }

      if (data_index.isEmpty())
      {
        data_index.resize(data_n);
        for (int i = 0; i < data_n; ++i)
        {
          if ((i+data_begin) >= 0 && (i+data_begin<n))
          {
            if (mask(i+data_begin))
              data_index(i) = i+data_begin;
            else
              data_index(i) = -1;
          }
          else
            data_index(i) = -1;
        }
      }
      else
      {
        if (data_index.numElements() != data_n)
        {
          ERROR("CAxis::checkData(void)",
                << "[ id = " << this->getId() << " , context = '" << CObjectFactory::GetCurrentContextId() << " ] "
                << "The size of data_index = "<< data_index.numElements() << "is not equal to the data size data_n = " << data_n.getValue() << ").");
        }
        for (int i = 0; i < data_n; ++i)
        {
           if (data_index(i) >= 0 && data_index(i)<n)
             if (!mask(data_index(i))) data_index(i) = -1;
        }
      }

   }
   CATCH_DUMP_ATTR

    size_t CAxis::getGlobalWrittenSize(void)
    {
      return n_glo ;
    }

   /*!
     Check validity of mask info and fill in values if any.
   */
   void CAxis::checkMask()
   TRY
   {
      if (!mask.isEmpty())
      {
        if (mask.extent(0) != n)
        {
          ERROR("CAxis::checkMask(void)",
              << "[ id = " << this->getId() << " , context = '" << CObjectFactory::GetCurrentContextId() << " ] "
              << "The mask does not have the same size as the local domain." << std::endl
              << "Local size is " << n.getValue() << "." << std::endl
              << "Mask size is " << mask.extent(0) << ".");
        }
      }
      else
      {
        mask.resize(n);
        mask = true;
      }
   }
   CATCH_DUMP_ATTR

   /*!
     Check validity of bounds info and fill in values if any.
   */
   void CAxis::checkBounds()
   TRY
   {
     if (!bounds.isEmpty())
     {
       if (bounds.extent(0) != 2 || bounds.extent(1) != n)
         ERROR("CAxis::checkAttributes(void)",
               << "The bounds array of the axis [ id = '" << getId() << "' , context = '" << CObjectFactory::GetCurrentContextId() << "' ] must be of dimension 2 x axis size." << std::endl
               << "Axis size is " << n.getValue() << "." << std::endl
               << "Bounds size is "<< bounds.extent(0) << " x " << bounds.extent(1) << ".");
       hasBounds = true;
     }
     else hasBounds = false;
   }
   CATCH_DUMP_ATTR

  void CAxis::checkLabel()
  TRY
  {
    if (!label.isEmpty())
    {
      if (label.extent(0) != n)
        ERROR("CAxis::checkLabel(void)",
              << "The label array of the axis [ id = '" << getId() << "' , context = '" << CObjectFactory::GetCurrentContextId() << "' ] must be of dimension of axis size." << std::endl
              << "Axis size is " << n.getValue() << "." << std::endl
              << "label size is "<< label.extent(0)<<  " .");
      hasLabel = true;
    }
    else hasLabel = false;
  }
  CATCH_DUMP_ATTR

 
  /*!
    Dispatch event from the lower communication layer then process event according to its type
  */
  bool CAxis::dispatchEvent(CEventServer& event)
  TRY
  {
     if (SuperClass::dispatchEvent(event)) return true;
     else
     {
       switch(event.type)
       {
          case EVENT_ID_DISTRIBUTION_ATTRIBUTE :
            recvDistributionAttribute(event);
            return true;
            break;
         case EVENT_ID_NON_DISTRIBUTED_ATTRIBUTES:
           recvNonDistributedAttributes(event);
           return true;
           break;
         case EVENT_ID_DISTRIBUTED_ATTRIBUTES:
           recvDistributedAttributes_old(event);
           return true;
           break;
         case EVENT_ID_AXIS_DISTRIBUTION:
           recvAxisDistribution(event);
           return true;
           break;
         case EVENT_ID_SEND_DISTRIBUTED_ATTRIBUTE:
           recvDistributedAttributes(event);
           return true;
           break;
          default :
            ERROR("bool CAxis::dispatchEvent(CEventServer& event)",
                   << "Unknown Event");
          return false;
        }
     }
  }
  CATCH

   /*!
     Check attributes on client side (This name is still adequate???)
   */
   void CAxis::checkAttributesOnClient()
   TRY
   {
     if (this->areClientAttributesChecked_) return;

     CContext* context=CContext::getCurrent();
     if (context->getServiceType()==CServicesManager::CLIENT) this->checkAttributes();

     this->areClientAttributesChecked_ = true;
   }
   CATCH_DUMP_ATTR

   /*
     The (spatial) transformation sometimes can change attributes of an axis (e.g zoom can change mask or generate can change whole attributes)
     Therefore, we should recheck them.
   */
   // ym : obsolete to be removed
   void CAxis::checkAttributesOnClientAfterTransformation(const std::vector<int>& globalDim, int orderPositionInGrid,
                                                          CServerDistributionDescription::ServerDistributionType distType)
   TRY
   {
     CContext* context=CContext::getCurrent() ;

     if (this->isClientAfterTransformationChecked) return;
     if (context->getServiceType()==CServicesManager::CLIENT || context->getServiceType()==CServicesManager::GATHERER)
     {        
       /* suppressed because of interface changed
       if (orderPositionInGrid == CServerDistributionDescription::defaultDistributedDimension(globalDim.size(), distType))
         computeConnectedClients(globalDim, orderPositionInGrid, distType);
       else if (index.numElements() != n_glo) computeConnectedClients(globalDim, orderPositionInGrid,  CServerDistributionDescription::ROOT_DISTRIBUTION);
       */
     }

     this->isClientAfterTransformationChecked = true;
   }
   CATCH_DUMP_ATTR

   /*
     Send all checked attributes to server? (We dont have notion of server any more so client==server)
     \param [in] globalDim global dimension of grid containing this axis
     \param [in] orderPositionInGrid the relative order of this axis in the grid (e.g grid composed of domain+axis -> orderPositionInGrid is 2)
     \param [in] distType distribution type of the server. For now, we only have band distribution.

   */
   //ym obsolete : to be removed
   void CAxis::sendCheckedAttributes(const std::vector<int>& globalDim, int orderPositionInGrid,
                                     CServerDistributionDescription::ServerDistributionType distType)
   TRY
   {
     if (!this->areClientAttributesChecked_) checkAttributesOnClient();
     if (!this->isClientAfterTransformationChecked) checkAttributesOnClientAfterTransformation(globalDim, orderPositionInGrid, distType);
     CContext* context = CContext::getCurrent();

     if (this->isChecked) return;
     if (context->getServiceType()==CServicesManager::CLIENT || context->getServiceType()==CServicesManager::GATHERER) /*sendAttributes(globalDim, orderPositionInGrid, distType)*/;    

     this->isChecked = true;
   }
   CATCH_DUMP_ATTR

  
   void CAxis::sendAxisToFileServer(CContextClient* client, const std::vector<int>& globalDim, int orderPositionInGrid)
   {
     if (sendAxisToFileServer_done_.count(client)!=0) return ;
     else sendAxisToFileServer_done_.insert(client) ;
     
     StdString axisDefRoot("axis_definition");
     CAxisGroup* axisPtr = CAxisGroup::get(axisDefRoot);
     axisPtr->sendCreateChild(this->getId(),client);
     this->sendAllAttributesToServer(client)  ; 
     this->sendAttributes(client, globalDim, orderPositionInGrid, CServerDistributionDescription::BAND_DISTRIBUTION) ;
   }

   void CAxis::sendAxisToCouplerOut(CContextClient* client, const std::vector<int>& globalDim, int orderPositionInGrid, const string& fieldId, int posInGrid)
   {
     if (sendAxisToFileServer_done_.count(client)!=0) return ;
     else sendAxisToFileServer_done_.insert(client) ;
     
     string axisId="_axis["+std::to_string(posInGrid)+"]_of_"+fieldId ;

     if (!axis_ref.isEmpty())
    {
      auto axis_ref_tmp=axis_ref.getValue() ;
      axis_ref.reset() ; // remove the reference, find an other way to do that more cleanly
      this->sendAllAttributesToServer(client, axisId)  ; 
      axis_ref = axis_ref_tmp ;
    }
    else this->sendAllAttributesToServer(client, axisId)  ; 
  
    this->sendAttributes(client, globalDim, orderPositionInGrid, CServerDistributionDescription::BAND_DISTRIBUTION, axisId) ;
   }

  void CAxis::makeAliasForCoupling(const string& fieldId, int posInGrid)
  {
    const string axisId = "_axis["+std::to_string(posInGrid)+"]_of_"+fieldId ;
    this->createAlias(axisId) ;
  }

  /*!
    Send attributes from one client to other clients
    \param[in] globalDim global dimension of grid which contains this axis
    \param[in] order
  */
  void CAxis::sendAttributes(CContextClient* client, const std::vector<int>& globalDim, int orderPositionInGrid,
                             CServerDistributionDescription::ServerDistributionType distType, const string& axisId)
  TRY
  {
     sendDistributionAttribute(client, globalDim, orderPositionInGrid, distType, axisId);

     // if (index.numElements() == n_glo.getValue())
     if ((orderPositionInGrid == CServerDistributionDescription::defaultDistributedDimension(globalDim.size(), distType))
         || (index.numElements() != n_glo))
     {
       sendDistributedAttributes_old(client, axisId);       
     }
     else
     {
       sendNonDistributedAttributes(client, axisId);    
     }     
  }
  CATCH_DUMP_ATTR

  /*
    Compute the connection between group of clients (or clients/servers).
    (E.g: Suppose we have 2 group of clients in two model: A (client role) connect to B (server role),
    this function calculate number of clients B connect to one client of A)
     \param [in] globalDim global dimension of grid containing this axis
     \param [in] orderPositionInGrid the relative order of this axis in the grid (e.g grid composed of domain+axis -> orderPositionInGrid is 2)
     \param [in] distType distribution type of the server. For now, we only have band distribution.
  */
  void CAxis::computeConnectedClients(CContextClient* client, const std::vector<int>& globalDim, int orderPositionInGrid)
  TRY
  {
    if (computeConnectedClients_done_.count(client)!=0) return ;
    else computeConnectedClients_done_.insert(client) ;

    CContext* context = CContext::getCurrent();
    CServerDistributionDescription::ServerDistributionType distType ;
    int defaultDistributedPos = CServerDistributionDescription::defaultDistributedDimension(globalDim.size(), CServerDistributionDescription::BAND_DISTRIBUTION) ;
    
    if (orderPositionInGrid == defaultDistributedPos) distType =  CServerDistributionDescription::BAND_DISTRIBUTION ;
    else if (index.numElements() != n_glo) distType =  CServerDistributionDescription::ROOT_DISTRIBUTION ;
    else return ;

    int nbServer = client->serverSize;
    int range, clientSize = client->clientSize;
    int rank = client->clientRank;

    if (listNbServer_.count(nbServer) == 0)
    {
      listNbServer_.insert(nbServer) ;

      if (connectedServerRank_.find(nbServer) != connectedServerRank_.end())
      {
        nbSenders.erase(nbServer);
        connectedServerRank_.erase(nbServer);
      }

      size_t ni = this->n.getValue();
      size_t ibegin = this->begin.getValue();
      size_t nbIndex = index.numElements();

      // First of all, we should compute the mapping of the global index and local index of the current client
      if (globalLocalIndexMap_.empty())
      {
        for (size_t idx = 0; idx < nbIndex; ++idx)
        {
          globalLocalIndexMap_[index(idx)] = idx;
        }
      }

      // Calculate the compressed index if any
      //        std::set<int> writtenInd;
      //        if (isCompressible_)
      //        {
      //          for (int idx = 0; idx < data_index.numElements(); ++idx)
      //          {
      //            int ind = CDistributionClient::getAxisIndex(data_index(idx), data_begin, ni); 
      //
      //            if (ind >= 0 && ind < ni && mask(ind))
      //            {
      //              ind += ibegin;
      //              writtenInd.insert(ind);
      //            }
      //          }
      //        }

      // Compute the global index of the current client (process) hold
      std::vector<int> nGlobAxis(1);
      nGlobAxis[0] = n_glo.getValue();

      size_t globalSizeIndex = 1, indexBegin, indexEnd;
      for (int i = 0; i < nGlobAxis.size(); ++i) globalSizeIndex *= nGlobAxis[i];
      indexBegin = 0;
      if (globalSizeIndex <= clientSize)
      {
        indexBegin = rank%globalSizeIndex;
        indexEnd = indexBegin;
      }
      else
      {
        for (int i = 0; i < clientSize; ++i)
        {
          range = globalSizeIndex / clientSize;
          if (i < (globalSizeIndex%clientSize)) ++range;
          if (i == client->clientRank) break;
          indexBegin += range;
        }
        indexEnd = indexBegin + range - 1;
      }

      CArray<size_t,1> globalIndex(index.numElements());
      for (size_t idx = 0; idx < globalIndex.numElements(); ++idx)
        globalIndex(idx) = index(idx);

      // Describe the distribution of server side

      CServerDistributionDescription serverDescription(nGlobAxis, nbServer, distType);
    
      std::vector<int> serverZeroIndex;
      serverZeroIndex = serverDescription.computeServerGlobalIndexInRange(std::make_pair<size_t&,size_t&>(indexBegin, indexEnd), 0);

      std::list<int> serverZeroIndexLeader;
      std::list<int> serverZeroIndexNotLeader; 
      CContextClient::computeLeader(client->clientRank, client->clientSize, serverZeroIndex.size(), serverZeroIndexLeader, serverZeroIndexNotLeader);
      for (std::list<int>::iterator it = serverZeroIndexLeader.begin(); it != serverZeroIndexLeader.end(); ++it)
        *it = serverZeroIndex[*it];

      // Find out the connection between client and server side
      CClientServerMapping* clientServerMap = new CClientServerMappingDistributed(serverDescription.getGlobalIndexRange(), client->intraComm);
      clientServerMap->computeServerIndexMapping(globalIndex, nbServer);
      CClientServerMapping::GlobalIndexMap& globalIndexAxisOnServer = clientServerMap->getGlobalIndexOnServer();      

      indSrv_[nbServer].swap(globalIndexAxisOnServer);

      if (distType==CServerDistributionDescription::ROOT_DISTRIBUTION)
      {
        for(int i=1; i<nbServer; ++i) indSrv_[nbServer].insert(pair<int, vector<size_t> >(i,indSrv_[nbServer][0]) ) ;
        serverZeroIndexLeader.clear() ;
      }
       
      CClientServerMapping::GlobalIndexMap::const_iterator it  = indSrv_[nbServer].begin(),
                                                           ite = indSrv_[nbServer].end();

      for (it = indSrv_[nbServer].begin(); it != ite; ++it) connectedServerRank_[nbServer].push_back(it->first);

      for (std::list<int>::const_iterator it = serverZeroIndexLeader.begin(); it != serverZeroIndexLeader.end(); ++it)
        connectedServerRank_[nbServer].push_back(*it);

       // Even if a client has no index, it must connect to at least one server and 
       // send an "empty" data to this server
       if (connectedServerRank_[nbServer].empty())
        connectedServerRank_[nbServer].push_back(client->clientRank % client->serverSize);

      nbSenders[nbServer] = CClientServerMapping::computeConnectedClients(client->serverSize, client->clientSize, client->intraComm, connectedServerRank_[nbServer]);

      delete clientServerMap;
    }
  }
  CATCH_DUMP_ATTR

  /*
    Compute the index of data to write into file
    (Different from the previous version, this version of XIOS allows data be written into file (classical role),
    or transfered to another clients)
  */
  void CAxis::computeWrittenIndex()
  TRY
  {  
    if (computedWrittenIndex_) return;
    computedWrittenIndex_ = true;

    CContext* context=CContext::getCurrent();      

    // We describe the distribution of client (server) on which data are written
    std::vector<int> nBegin(1), nSize(1), nBeginGlobal(1), nGlob(1);
    nBegin[0]       = begin;
    nSize[0]        = n;
    nBeginGlobal[0] = 0; 
    nGlob[0]        = n_glo;
    CDistributionServer srvDist(context->intraCommSize_, nBegin, nSize, nBeginGlobal, nGlob); 
    const CArray<size_t,1>& writtenGlobalIndex  = srvDist.getGlobalIndex();

    // Because all written data are local on a client, 
    // we need to compute the local index on the server from its corresponding global index
    size_t nbWritten = 0, indGlo;      
    std::unordered_map<size_t,size_t>::const_iterator itb = globalLocalIndexMap_.begin(),
                                                        ite = globalLocalIndexMap_.end(), it;          
    CArray<size_t,1>::const_iterator itSrvb = writtenGlobalIndex.begin(),
                                     itSrve = writtenGlobalIndex.end(), itSrv;  

    localIndexToWriteOnServer.resize(writtenGlobalIndex.numElements());
    nbWritten = 0;
    for (itSrv = itSrvb; itSrv != itSrve; ++itSrv)
    {
      indGlo = *itSrv;
      if (ite != globalLocalIndexMap_.find(indGlo))
      {
        localIndexToWriteOnServer(nbWritten) = globalLocalIndexMap_[indGlo];
      }
      else
      {
        localIndexToWriteOnServer(nbWritten) = -1;
      }
      ++nbWritten;
    }

  }
  CATCH_DUMP_ATTR

  void CAxis::computeWrittenCompressedIndex(MPI_Comm writtenComm)
  TRY
  {
    int writtenCommSize;
    MPI_Comm_size(writtenComm, &writtenCommSize);
    if (compressedIndexToWriteOnServer.find(writtenCommSize) != compressedIndexToWriteOnServer.end())
      return;

    if (isCompressible())
    {
      size_t nbWritten = 0, indGlo;
      CContext* context=CContext::getCurrent();      
 
      // We describe the distribution of client (server) on which data are written
      std::vector<int> nBegin(1), nSize(1), nBeginGlobal(1), nGlob(1);
      nBegin[0]       = 0;
      nSize[0]        = n;
      nBeginGlobal[0] = 0; 
      nGlob[0]        = n_glo;
      CDistributionServer srvDist(context->intraCommSize_, nBegin, nSize, nBeginGlobal, nGlob); 
      const CArray<size_t,1>& writtenGlobalIndex  = srvDist.getGlobalIndex();
      std::unordered_map<size_t,size_t>::const_iterator itb = globalLocalIndexMap_.begin(),
                                                          ite = globalLocalIndexMap_.end(), it;   

      CArray<size_t,1>::const_iterator itSrvb = writtenGlobalIndex.begin(),
                                       itSrve = writtenGlobalIndex.end(), itSrv;
      std::unordered_map<size_t,size_t> localGlobalIndexMap;
      for (itSrv = itSrvb; itSrv != itSrve; ++itSrv)
      {
        indGlo = *itSrv;
        if (ite != globalLocalIndexMap_.find(indGlo))
        {
          localGlobalIndexMap[localIndexToWriteOnServer(nbWritten)] = indGlo;
          ++nbWritten;
        }                 
      }
//
//      nbWritten = 0;
//      for (int idx = 0; idx < data_index.numElements(); ++idx)
//      {
//        if (localGlobalIndexMap.end() != localGlobalIndexMap.find(data_index(idx)))
//        {
//          ++nbWritten;
//        }
//      }
//
//      compressedIndexToWriteOnServer[writtenCommSize].resize(nbWritten);
//      nbWritten = 0;
//      for (int idx = 0; idx < data_index.numElements(); ++idx)
//      {
//        if (localGlobalIndexMap.end() != localGlobalIndexMap.find(data_index(idx)))
//        {
//          compressedIndexToWriteOnServer[writtenCommSize](nbWritten) = localGlobalIndexMap[data_index(idx)];
//          ++nbWritten;
//        }
//      }

      nbWritten = 0;
      for (int idx = 0; idx < data_index.numElements(); ++idx)
      {
        if (localGlobalIndexMap.end() != localGlobalIndexMap.find(data_index(idx)))
        {
          ++nbWritten;
        }
      }

      compressedIndexToWriteOnServer[writtenCommSize].resize(nbWritten);
      nbWritten = 0;
      for (int idx = 0; idx < data_index.numElements(); ++idx)
      {
        if (localGlobalIndexMap.end() != localGlobalIndexMap.find(data_index(idx)))
        {
          compressedIndexToWriteOnServer[writtenCommSize](nbWritten) = localGlobalIndexMap[data_index(idx)];
          ++nbWritten;
        }
      }

      numberWrittenIndexes_[writtenCommSize] = nbWritten;

      bool distributed_glo, distributed=isDistributed() ;
      MPI_Allreduce(&distributed,&distributed_glo, 1, MPI_INT, MPI_LOR, writtenComm) ;
      if (distributed_glo)
      {
             
        MPI_Allreduce(&numberWrittenIndexes_[writtenCommSize], &totalNumberWrittenIndexes_[writtenCommSize], 1, MPI_INT, MPI_SUM, writtenComm);
        MPI_Scan(&numberWrittenIndexes_[writtenCommSize], &offsetWrittenIndexes_[writtenCommSize], 1, MPI_INT, MPI_SUM, writtenComm);
        offsetWrittenIndexes_[writtenCommSize] -= numberWrittenIndexes_[writtenCommSize];
      }
      else
        totalNumberWrittenIndexes_[writtenCommSize] = numberWrittenIndexes_[writtenCommSize];
    }
  }
  CATCH_DUMP_ATTR

  /*!
    Send distribution information from a group of client (client role) to another group of client (server role)
    The distribution of a group of client (server role) is imposed by the group of client (client role)
    \param [in] globalDim global dimension of grid containing this axis
    \param [in] orderPositionInGrid the relative order of this axis in the grid (e.g grid composed of domain+axis -> orderPositionInGrid is 2)
    \param [in] distType distribution type of the server. For now, we only have band distribution.
  */
  void CAxis::sendDistributionAttribute(CContextClient* client, const std::vector<int>& globalDim, int orderPositionInGrid,
                                        CServerDistributionDescription::ServerDistributionType distType, const string& axisId)
  TRY
  {
    string serverAxisId = axisId.empty() ? this->getId() : axisId ; 
    int nbServer = client->serverSize;

    CServerDistributionDescription serverDescription(globalDim, nbServer);
    serverDescription.computeServerDistribution();

    std::vector<std::vector<int> > serverIndexBegin = serverDescription.getServerIndexBegin();
    std::vector<std::vector<int> > serverDimensionSizes = serverDescription.getServerDimensionSizes();

    CEventClient event(getType(),EVENT_ID_DISTRIBUTION_ATTRIBUTE);
    if (client->isServerLeader())
    {
      std::list<CMessage> msgs;

      const std::list<int>& ranks = client->getRanksServerLeader();
      for (std::list<int>::const_iterator itRank = ranks.begin(), itRankEnd = ranks.end(); itRank != itRankEnd; ++itRank)
      {
        // Use const int to ensure CMessage holds a copy of the value instead of just a reference
        const int begin = serverIndexBegin[*itRank][orderPositionInGrid];
        const int ni    = serverDimensionSizes[*itRank][orderPositionInGrid];

        msgs.push_back(CMessage());
        CMessage& msg = msgs.back();
        msg << serverAxisId;
        msg << ni << begin;
        msg << isCompressible_;                    

        event.push(*itRank,1,msg);
      }
      client->sendEvent(event);
    }
    else client->sendEvent(event);
  }
  CATCH_DUMP_ATTR

  /*
    Receive distribution attribute from another client
    \param [in] event event containing data of these attributes
  */
  void CAxis::recvDistributionAttribute(CEventServer& event)
  TRY
  {
    CBufferIn* buffer = event.subEvents.begin()->buffer;
    string axisId;
    *buffer >> axisId;
    get(axisId)->recvDistributionAttribute(*buffer);
  }
  CATCH

  /*
    Receive distribution attribute from another client
    \param [in] buffer buffer containing data of these attributes
  */
  void CAxis::recvDistributionAttribute(CBufferIn& buffer)
  TRY
  {
    int ni_srv, begin_srv;
    buffer >> ni_srv >> begin_srv;
    buffer >> isCompressible_;            

    // Set up new local size of axis on the receiving clients
    n.setValue(ni_srv);
    begin.setValue(begin_srv);
  }
  CATCH_DUMP_ATTR

  /*
    Send attributes of axis from a group of client to other group of clients/servers 
    on supposing that these attributes are not distributed among the sending group
    In the future, if new attributes are added, they should also be processed in this function
  */
  void CAxis::sendNonDistributedAttributes(CContextClient* client, const string& axisId)
  TRY
  {
    string serverAxisId = axisId.empty() ? this->getId() : axisId ; 

    CEventClient event(getType(), EVENT_ID_NON_DISTRIBUTED_ATTRIBUTES);
    size_t nbIndex = index.numElements();
    size_t nbDataIndex = 0;

    for (int idx = 0; idx < data_index.numElements(); ++idx)
    {
      int ind = data_index(idx);
      if (ind >= 0 && ind < nbIndex) ++nbDataIndex;
    }

    CArray<int,1> dataIndex(nbDataIndex);
    nbDataIndex = 0;
    for (int idx = 0; idx < data_index.numElements(); ++idx)
    {
      int ind = data_index(idx);
      if (ind >= 0 && ind < nbIndex)
      {
        dataIndex(nbDataIndex) = ind;
        ++nbDataIndex;
      }
    }

    if (client->isServerLeader())
    {
      std::list<CMessage> msgs;

      const std::list<int>& ranks = client->getRanksServerLeader();
      for (std::list<int>::const_iterator itRank = ranks.begin(), itRankEnd = ranks.end(); itRank != itRankEnd; ++itRank)
      {
        msgs.push_back(CMessage());
        CMessage& msg = msgs.back();
        msg << serverAxisId;
        msg << index.getValue() << dataIndex << mask.getValue();
        msg << hasValue;
        if (hasValue) msg << value.getValue();
        msg << hasBounds;
        if (hasBounds) msg << bounds.getValue();
        msg << hasLabel;
        if (hasLabel) msg << label.getValue();

        event.push(*itRank, 1, msg);
      }
      client->sendEvent(event);
    }
    else client->sendEvent(event);
  }
  CATCH_DUMP_ATTR

  /*
    Receive the non-distributed attributes from another group of clients
    \param [in] event event containing data of these attributes
  */
  void CAxis::recvNonDistributedAttributes(CEventServer& event)
  TRY
  {
    list<CEventServer::SSubEvent>::iterator it;
    for (it = event.subEvents.begin(); it != event.subEvents.end(); ++it)
    {
      CBufferIn* buffer = it->buffer;
      string axisId;
      *buffer >> axisId;
      get(axisId)->recvNonDistributedAttributes(it->rank, *buffer);
    }
  }
  CATCH

  /*
    Receive the non-distributed attributes from another group of clients
    \param [in] rank rank of the sender
    \param [in] buffer buffer containing data sent from the sender
  */
  void CAxis::recvNonDistributedAttributes(int rank, CBufferIn& buffer)
  TRY
  { 
    CArray<int,1> tmp_index, tmp_data_index;
    CArray<bool,1> tmp_mask;
    CArray<double,1> tmp_val;
    CArray<double,2> tmp_bnds;
    CArray<string,1> tmp_label;

    buffer >> tmp_index;
    index.reference(tmp_index);
    buffer >> tmp_data_index;
    data_index.reference(tmp_data_index);
    buffer >> tmp_mask;
    mask.reference(tmp_mask);

    buffer >> hasValue;
    if (hasValue)
    {
      buffer >> tmp_val;
      value.reference(tmp_val);
    }

    buffer >> hasBounds;
    if (hasBounds)
    {
      buffer >> tmp_bnds;
      bounds.reference(tmp_bnds);
    }

    buffer >> hasLabel;
    if (hasLabel)
    {
      buffer >> tmp_label;
      label.reference(tmp_label);
    }

    // Some value should be reset here
    data_begin.setValue(0);
    data_n.setValue(data_index.numElements());
    globalLocalIndexMap_.rehash(std::ceil(index.numElements()/globalLocalIndexMap_.max_load_factor()));
//    for (int idx = 0; idx < index.numElements(); ++idx) globalLocalIndexMap_[idx] = index(idx);
    for (int idx = 0; idx < index.numElements(); ++idx) globalLocalIndexMap_[index(idx)] = idx;
  }
  CATCH_DUMP_ATTR

  /*
    Send axis attributes from a group of clients to another group of clients/servers
    supposing that these attributes are distributed among the clients of the sending group
    In future, if new attributes are added, they should also be processed in this function
  */
  void CAxis::sendDistributedAttributes_old(CContextClient* client, const string& axisId)
  TRY
  {
    string serverAxisId = axisId.empty() ? this->getId() : axisId ; 
    
    int ind, idx;
    int nbServer = client->serverSize;

    CEventClient eventData(getType(), EVENT_ID_DISTRIBUTED_ATTRIBUTES);

    list<CMessage> listData;
    list<CArray<int,1> > list_indi, list_dataInd;
    list<CArray<double,1> > list_val;
    list<CArray<double,2> > list_bounds;
    list<CArray<string,1> > list_label;

    // Cut off the ghost points
    int nbIndex = index.numElements();
    CArray<int,1> dataIndex(nbIndex);
    dataIndex = -1;
    for (idx = 0; idx < data_index.numElements(); ++idx)
    {
      if (0 <= data_index(idx) && data_index(idx) < nbIndex)
        dataIndex(data_index(idx)) = 1;
    }

    std::unordered_map<int, std::vector<size_t> >::const_iterator it, iteMap;
    iteMap = indSrv_[nbServer].end();
    for (int k = 0; k < connectedServerRank_[nbServer].size(); ++k)
    {
      int nbData = 0, nbDataCount = 0;
      int rank = connectedServerRank_[nbServer][k];
      it = indSrv_[nbServer].find(rank);
      if (iteMap != it)
        nbData = it->second.size();

      list_indi.push_back(CArray<int,1>(nbData));
      list_dataInd.push_back(CArray<int,1>(nbData));

      if (hasValue)
        list_val.push_back(CArray<double,1>(nbData));

      if (hasBounds)        
        list_bounds.push_back(CArray<double,2>(2,nbData));

      if (hasLabel)
        list_label.push_back(CArray<string,1>(nbData));

      CArray<int,1>& indi = list_indi.back();
      CArray<int,1>& dataIndi = list_dataInd.back();
      dataIndi = -1;

      for (int n = 0; n < nbData; ++n)
      {
        idx = static_cast<int>(it->second[n]);
        indi(n) = idx;

        ind = globalLocalIndexMap_[idx];
        dataIndi(n) = dataIndex(ind);

        if (hasValue)
        {
          CArray<double,1>& val = list_val.back();
          val(n) = value(ind);
        }

        if (hasBounds)
        {
          CArray<double,2>& boundsVal = list_bounds.back();
          boundsVal(0, n) = bounds(0,ind);
          boundsVal(1, n) = bounds(1,ind);
        }

        if (hasLabel)
        {
          CArray<string,1>& labelVal = list_label.back();
          labelVal(n) = label(ind); 
        }
      }

      listData.push_back(CMessage());
      listData.back() << serverAxisId
                      << list_indi.back() << list_dataInd.back();

      listData.back() << hasValue;
      if (hasValue)
        listData.back() << list_val.back();

      listData.back() << hasBounds;
      if (hasBounds)
        listData.back() << list_bounds.back();

      listData.back() << hasLabel;
      if (hasLabel)
        listData.back() << list_label.back();

      eventData.push(rank, nbSenders[nbServer][rank], listData.back());
    }

    client->sendEvent(eventData);
  }
  CATCH_DUMP_ATTR

  /*
    Receive the distributed attributes from another group of clients
    \param [in] event event containing data of these attributes
  */
  void CAxis::recvDistributedAttributes_old(CEventServer& event)
  TRY
  {
    string axisId;
    vector<int> ranks;
    vector<CBufferIn*> buffers;

    list<CEventServer::SSubEvent>::iterator it;
    for (it = event.subEvents.begin(); it != event.subEvents.end(); ++it)
    {
      ranks.push_back(it->rank);
      CBufferIn* buffer = it->buffer;
      *buffer >> axisId;
      buffers.push_back(buffer);
    }
    get(axisId)->recvDistributedAttributes_old(ranks, buffers);
  }
  CATCH

  /*
    Receive the non-distributed attributes from another group of clients
    \param [in] ranks rank of the sender
    \param [in] buffers buffer containing data sent from the sender
  */
  void CAxis::recvDistributedAttributes_old(vector<int>& ranks, vector<CBufferIn*> buffers)
  TRY
  {
    int nbReceived = ranks.size(), idx, ind, gloInd, locInd;
    vector<CArray<int,1> > vec_indi(nbReceived), vec_dataInd(nbReceived);
    vector<CArray<double,1> > vec_val(nbReceived);
    vector<CArray<double,2> > vec_bounds(nbReceived);
    vector<CArray<string,1> > vec_label(nbReceived);
    
    for (idx = 0; idx < nbReceived; ++idx)
    {      
      CBufferIn& buffer = *buffers[idx];
      buffer >> vec_indi[idx];
      buffer >> vec_dataInd[idx];      

      buffer >> hasValue;
      if (hasValue)
        buffer >> vec_val[idx];

      buffer >> hasBounds;
      if (hasBounds)
        buffer >> vec_bounds[idx];

      buffer >> hasLabel;
      if (hasLabel)
        buffer >> vec_label[idx]; 
    }

    // Estimate size of index array
    int nbIndexGlob = 0;
    for (idx = 0; idx < nbReceived; ++idx)
    {
      nbIndexGlob += vec_indi[idx].numElements();
    }

    // Recompute global index
    // Take account of the overlapped index 
    index.resize(nbIndexGlob);
    globalLocalIndexMap_.rehash(std::ceil(index.numElements()/globalLocalIndexMap_.max_load_factor()));
    nbIndexGlob = 0;
    int nbIndLoc = 0;
    for (idx = 0; idx < nbReceived; ++idx)
    {
      CArray<int,1>& tmp = vec_indi[idx];
      for (ind = 0; ind < tmp.numElements(); ++ind)
      {
         gloInd = tmp(ind);
         nbIndLoc = (gloInd % n_glo)-begin;
         if (0 == globalLocalIndexMap_.count(gloInd))
         {
           index(nbIndexGlob) = gloInd % n_glo;
           globalLocalIndexMap_[gloInd] = nbIndexGlob;
           ++nbIndexGlob;
         } 
      } 
    }

    // Resize index to its real size
    if (nbIndexGlob==0) index.resize(nbIndexGlob) ;
    else index.resizeAndPreserve(nbIndexGlob);

    int nbData = nbIndexGlob;
    CArray<int,1> nonCompressedData(nbData);
    nonCompressedData = -1;   
    // Mask is incorporated into data_index and is not sent/received anymore
    mask.reset();
    if (hasValue)
      value.resize(nbData);
    if (hasBounds)
      bounds.resize(2,nbData);
    if (hasLabel)
      label.resize(nbData);

    nbData = 0;
    for (idx = 0; idx < nbReceived; ++idx)
    {
      CArray<int,1>& indi = vec_indi[idx];
      CArray<int,1>& dataIndi = vec_dataInd[idx];
      int nb = indi.numElements();
      for (int n = 0; n < nb; ++n)
      { 
        locInd = globalLocalIndexMap_[size_t(indi(n))];

        nonCompressedData(locInd) = (-1 == nonCompressedData(locInd)) ? dataIndi(n) : nonCompressedData(locInd);

        if (hasValue)
          value(locInd) = vec_val[idx](n);

        if (hasBounds)
        {
          bounds(0,locInd) = vec_bounds[idx](0,n);
          bounds(1,locInd) = vec_bounds[idx](1,n);
        }

        if (hasLabel)
          label(locInd) = vec_label[idx](n);
      }
    }
    
    int nbCompressedData = 0;
    for (idx = 0; idx < nonCompressedData.numElements(); ++idx)
    {
      if (0 <= nonCompressedData(idx))
        ++nbCompressedData;
    }

    data_index.resize(nbCompressedData);
    nbCompressedData = 0;
    for (idx = 0; idx < nonCompressedData.numElements(); ++idx)
    {
      if (0 <= nonCompressedData(idx))
      {
        data_index(nbCompressedData) = idx % n;
        ++nbCompressedData;
      }
    }

    data_begin.setValue(0);
    data_n.setValue(data_index.numElements());
  }
  CATCH_DUMP_ATTR

  /*!
    Compare two axis objects. 
    They are equal if only if they have identical attributes as well as their values.
    Moreover, they must have the same transformations.
  \param [in] axis Compared axis
  \return result of the comparison
  */
  bool CAxis::isEqual(CAxis* obj)
  TRY
  {
    vector<StdString> excludedAttr;
    excludedAttr.push_back("axis_ref");

    bool objEqual = SuperClass::isEqual(obj, excludedAttr);    
    if (!objEqual) return objEqual;

    TransMapTypes thisTrans = this->getAllTransformations();
    TransMapTypes objTrans  = obj->getAllTransformations();

    TransMapTypes::const_iterator it, itb, ite;
    std::vector<ETranformationType> thisTransType, objTransType;
    for (it = thisTrans.begin(); it != thisTrans.end(); ++it)
      thisTransType.push_back(it->first);
    for (it = objTrans.begin(); it != objTrans.end(); ++it)
      objTransType.push_back(it->first);

    if (thisTransType.size() != objTransType.size()) return false;
    for (int idx = 0; idx < thisTransType.size(); ++idx)
      objEqual &= (thisTransType[idx] == objTransType[idx]);

    return objEqual;
  }
  CATCH_DUMP_ATTR

  /*
    Add transformation into axis. This function only servers for Fortran interface
    \param [in] transType transformation type
    \param [in] id identifier of the transformation object
  */
  CTransformation<CAxis>* CAxis::addTransformation(ETranformationType transType, const StdString& id)
  TRY
  {
    transformationMap_.push_back(std::make_pair(transType, CTransformation<CAxis>::createTransformation(transType,id)));
    return transformationMap_.back().second;
  }
  CATCH_DUMP_ATTR

  /*
    Check whether an axis has (spatial) transformation
  */
  bool CAxis::hasTransformation()
  TRY
  {
    return (!transformationMap_.empty());
  }
  CATCH_DUMP_ATTR

  /*
    Set transformation
    \param [in] axisTrans transformation to set
  */
  void CAxis::setTransformations(const TransMapTypes& axisTrans)
  TRY
  {
    transformationMap_ = axisTrans;
  }
  CATCH_DUMP_ATTR

  /*
    Return all transformation held by the axis
    \return transformation the axis has
  */
  CAxis::TransMapTypes CAxis::getAllTransformations(void)
  TRY
  {
    return transformationMap_;
  }
  CATCH_DUMP_ATTR

  /*
    Duplicate transformation of another axis
    \param [in] src axis whose transformations are copied
  */
  void CAxis::duplicateTransformation(CAxis* src)
  TRY
  {
    if (src->hasTransformation())
    {
      this->setTransformations(src->getAllTransformations());
    }
  }
  CATCH_DUMP_ATTR

  /*!
   * Go through the hierarchy to find the axis from which the transformations must be inherited
   */
  void CAxis::solveInheritanceTransformation()
  TRY
  {
    if (hasTransformation() || !hasDirectAxisReference())
      return;

    CAxis* axis = this;
    std::vector<CAxis*> refAxis;
    while (!axis->hasTransformation() && axis->hasDirectAxisReference())
    {
      refAxis.push_back(axis);
      axis = axis->getDirectAxisReference();
    }

    if (axis->hasTransformation())
      for (size_t i = 0; i < refAxis.size(); ++i)
        refAxis[i]->setTransformations(axis->getAllTransformations());
  }
  CATCH_DUMP_ATTR

  void CAxis::setContextClient(CContextClient* contextClient)
  TRY
  {
    if (clientsSet.find(contextClient)==clientsSet.end())
    {
      clients.push_back(contextClient) ;
      clientsSet.insert(contextClient);
    }
  }
  CATCH_DUMP_ATTR

  void CAxis::parse(xml::CXMLNode & node)
  TRY
  {
    SuperClass::parse(node);

    if (node.goToChildElement())
    {
      StdString nodeElementName;
      do
      {
        StdString nodeId("");
        if (node.getAttributes().end() != node.getAttributes().find("id"))
        { nodeId = node.getAttributes()["id"]; }

        nodeElementName = node.getElementName();
        std::map<StdString, ETranformationType>::const_iterator ite = transformationMapList_.end(), it;
        it = transformationMapList_.find(nodeElementName);
        if (ite != it)
        {
          transformationMap_.push_back(std::make_pair(it->second, CTransformation<CAxis>::createTransformation(it->second,
                                                                                                               nodeId,
                                                                                                               &node)));
        }
        else
        {
          ERROR("void CAxis::parse(xml::CXMLNode & node)",
                << "The transformation " << nodeElementName << " has not been supported yet.");
        }
      } while (node.goToNextElement()) ;
      node.goToParentElement();
    }
  }
  CATCH_DUMP_ATTR


   //////////////////////////////////////////////////////////////////////////////////////
   //  this part is related to distribution, element definition, views and connectors  //
   //////////////////////////////////////////////////////////////////////////////////////

   void CAxis::initializeLocalElement(void)
   {
      // after checkAttribute index of size n
      int rank = CContext::getCurrent()->getIntraCommRank() ;
      
      CArray<size_t,1> ind(n) ;
      for (int i=0;i<n;i++) ind(i)=index(i) ;

      localElement_ = new CLocalElement(rank, n_glo, ind) ;
   }

   void CAxis::addFullView(void)
   {
      CArray<int,1> index(n) ;
      for(int i=0; i<n ; i++) index(i)=i ;
      localElement_ -> addView(CElementView::FULL, index) ;
   }

   void CAxis::addWorkflowView(void)
   {
     // mask + data are included into data_index
     int nk=data_index.numElements() ;
     int nMask=0 ;
     for(int k=0;k<nk;k++) if (data_index(k)>=0 && data_index(k)<n) nMask++ ;
     
     CArray<int,1> index(nMask) ;
     nMask=0 ;
     for(int k=0;k<nk;k++) 
       if (data_index(k)>=0 && data_index(k)<n) 
       {
         index(nMask) = data_index(k) ;
         nMask++ ;
       }
     localElement_ -> addView(CElementView::WORKFLOW, index) ;
   }

   void CAxis::addModelView(void)
   {
     // information for model view is stored in data_index
     localElement_->addView(CElementView::MODEL, data_index) ;
   }

   void CAxis::computeModelToWorkflowConnector(void)
   { 
     CLocalView* srcView=getLocalView(CElementView::MODEL) ;
     CLocalView* dstView=getLocalView(CElementView::WORKFLOW) ;
     modelToWorkflowConnector_ = new CLocalConnector(srcView, dstView); 
     modelToWorkflowConnector_->computeConnector() ;
   }


   void CAxis::computeRemoteElement(CContextClient* client, EDistributionType type)
  {
    CContext* context = CContext::getCurrent();
    map<int, CArray<size_t,1>> globalIndex ;

    if (type==EDistributionType::BANDS) // Bands distribution to send to file server
    {
      int nbServer = client->serverSize;
      int nbClient = client->clientSize ;
      int rankClient = client->clientRank ;
      int size = nbServer / nbClient ;
      int start ;
      if (nbServer%nbClient > rankClient)
      {
       start = (size+1) * rankClient ;
       size++ ;
      }
      else start = size*rankClient + nbServer%nbClient ;
     
      for(int i=0; i<size; i++)
      { 
        int rank=start+i ; 
        size_t indSize = n_glo/nbServer ;
        size_t indStart ;
        if (n_glo % nbServer > rank)
        {
          indStart = (indSize+1) * rank ;
          indSize++ ;
        }
        else indStart = indSize*rank + n_glo%nbServer ;
       
        auto& globalInd =  globalIndex[rank] ;
        globalInd.resize(indSize) ;
        for(size_t n = 0 ; n<indSize; n++) globalInd(n)=indStart+n ;
      }
    }
    else if (type==EDistributionType::NONE) // domain is not distributed ie all servers get the same local domain
    {
      int nbServer = client->serverSize;
      size_t nglo=n_glo ;
      CArray<size_t,1> indGlo(nglo) ;
      for(size_t i=0;i<nglo;i++) indGlo(i) = i ;
      for (auto& rankServer : client->getRanksServerLeader()) globalIndex[rankServer].reference(indGlo.copy()); 
    }
    remoteElement_[client] = new CDistributedElement(n_glo, globalIndex) ;
    remoteElement_[client]->addFullView() ;
  }
 
  void CAxis::distributeToServer(CContextClient* client, std::map<int, CArray<size_t,1>>& globalIndex, 
                                 CScattererConnector* &scattererConnector, const string& axisId)
  {
    string serverAxisId = axisId.empty() ? this->getId() : axisId ;
    CContext* context = CContext::getCurrent();

    this->sendAllAttributesToServer(client, serverAxisId)  ;

    CDistributedElement scatteredElement(n_glo,globalIndex) ;
    scatteredElement.addFullView() ;
    scattererConnector = new CScattererConnector(localElement_->getView(CElementView::FULL), scatteredElement.getView(CElementView::FULL), 
                                                 context->getIntraComm(), client->getRemoteSize()) ;
    scattererConnector->computeConnector() ;
    
    // phase 0
    // send remote element to construct the full view on server, ie without hole 
    CEventClient event0(getType(), EVENT_ID_AXIS_DISTRIBUTION);
    CMessage message0 ;
    message0<<serverAxisId<<0 ; 
    remoteElement_[client]->sendToServer(client,event0,message0) ; 
    
    // phase 1
    // send the full view of element to construct the connector which connect distributed data coming from client to the full local view
    CEventClient event1(getType(), EVENT_ID_AXIS_DISTRIBUTION);
    CMessage message1 ;
    message1<<serverAxisId<<1<<localElement_->getView(CElementView::FULL)->getGlobalSize() ; 
    scattererConnector->transfer(localElement_->getView(CElementView::FULL)->getGlobalIndex(),client,event1,message1) ;

    sendDistributedAttributes(client, *scattererConnector, axisId) ;
  
    // phase 2 send the mask : data index + mask2D
    CArray<bool,1> maskIn(localElement_->getView(CElementView::WORKFLOW)->getSize());
    CArray<bool,1> maskOut ;
    CLocalConnector workflowToFull(localElement_->getView(CElementView::WORKFLOW), localElement_->getView(CElementView::FULL)) ;
    workflowToFull.computeConnector() ;
    maskIn=true ;
    workflowToFull.transfer(maskIn,maskOut,false) ;

    // phase 3 : prepare grid scatterer connector to send data from client to server
    map<int,CArray<size_t,1>> workflowGlobalIndex ;
    map<int,CArray<bool,1>> maskOut2 ; 
    scattererConnector->transfer(maskOut, maskOut2) ;
    scatteredElement.addView(CElementView::WORKFLOW, maskOut2) ;
    scatteredElement.getView(CElementView::WORKFLOW)->getGlobalIndexView(workflowGlobalIndex) ;
    // create new workflow view for scattered element
    CDistributedElement clientToServerElement(scatteredElement.getGlobalSize(), workflowGlobalIndex) ;
    clientToServerElement.addFullView() ;
    CEventClient event2(getType(), EVENT_ID_AXIS_DISTRIBUTION);
    CMessage message2 ;
    message2<<serverAxisId<<2 ; 
    clientToServerElement.sendToServer(client, event2, message2) ; 
    clientToServerConnector_[client] = new CScattererConnector(localElement_->getView(CElementView::WORKFLOW), clientToServerElement.getView(CElementView::FULL), 
                                                              context->getIntraComm(), client->getRemoteSize()) ;
    clientToServerConnector_[client]->computeConnector() ;

    clientFromServerConnector_[client] = new CGathererConnector(clientToServerElement.getView(CElementView::FULL), localElement_->getView(CElementView::WORKFLOW));
    clientFromServerConnector_[client]->computeConnector() ;


  }

  void CAxis::recvAxisDistribution(CEventServer& event)
  TRY
  {
    string axisId;
    int phasis ;
    for (auto& subEvent : event.subEvents) (*subEvent.buffer) >> axisId >> phasis ;
    get(axisId)->receivedAxisDistribution(event, phasis);
  }
  CATCH


  void CAxis::receivedAxisDistribution(CEventServer& event, int phasis)
  TRY
  {
    CContext* context = CContext::getCurrent();
    if (phasis==0) // receive the remote element to construct the full view
    {
      localElement_ = new  CLocalElement(context->getIntraCommRank(),event) ;
      localElement_->addFullView() ;
      // construct the local dimension and indexes
      auto& globalIndex=localElement_->getGlobalIndex() ;
      int nk=globalIndex.numElements() ;
      int minK=n_glo,maxK=-1 ;
      int nGlo=n_glo ;
      int indGlo ;
      for(int k=0;k<nk;k++)
      {
        indGlo=globalIndex(k) ;
        if (indGlo<minK) minK=indGlo ;
        if (indGlo>maxK) maxK=indGlo ;
      }  
      if (maxK>=minK) { begin=minK ; n=maxK-minK+1 ; }
      else {begin=0; n=0 ;}

    }
    else if (phasis==1) // receive the sent view from client to construct the full distributed full view on server
    {
      CContext* context = CContext::getCurrent();
      CDistributedElement* elementFrom = new  CDistributedElement(event) ;
      elementFrom->addFullView() ;
      gathererConnector_ = new CGathererConnector(elementFrom->getView(CElementView::FULL), localElement_->getView(CElementView::FULL)) ;
      gathererConnector_->computeConnector() ; 
    }
    else if (phasis==2)
    {
//      delete gathererConnector_ ;
      elementFrom_ = new  CDistributedElement(event) ;
      elementFrom_->addFullView() ;
//      gathererConnector_ =  new CGathererConnector(elementFrom_->getView(CElementView::FULL), localElement_->getView(CElementView::FULL)) ;
//      gathererConnector_ -> computeConnector() ;
    }
  
  }
  CATCH

  void CAxis::setServerMask(CArray<bool,1>& serverMask, CContextClient* client)
  TRY
  {
    CContext* context = CContext::getCurrent();
    localElement_->addView(CElementView::WORKFLOW, serverMask) ;
    mask.reference(serverMask.copy()) ;
 
    serverFromClientConnector_ = new CGathererConnector(elementFrom_->getView(CElementView::FULL), localElement_->getView(CElementView::WORKFLOW)) ;
    serverFromClientConnector_->computeConnector() ;
      
    serverToClientConnector_ = new CScattererConnector(localElement_->getView(CElementView::WORKFLOW), elementFrom_->getView(CElementView::FULL),
                                                         context->getIntraComm(), client->getRemoteSize()) ;
    serverToClientConnector_->computeConnector() ;
  }
  CATCH_DUMP_ATTR

  void CAxis::sendDistributedAttributes(CContextClient* client, CScattererConnector& scattererConnector, const string& axisId)
  {
    string serverAxisId = axisId.empty() ? this->getId() : axisId ;
    CContext* context = CContext::getCurrent();

    if (hasValue)
    {
      { // send level value
        CEventClient event(getType(), EVENT_ID_SEND_DISTRIBUTED_ATTRIBUTE);
        CMessage message ;
        message<<serverAxisId<<string("value") ; 
        scattererConnector.transfer(value, client, event,message) ;
      }
    }

    if (hasBounds)
    {
      { // send bounds level value
        CEventClient event(getType(), EVENT_ID_SEND_DISTRIBUTED_ATTRIBUTE);
        CMessage message ;
        message<<serverAxisId<<string("bounds") ; 
        scattererConnector.transfer(2, bounds, client, event,message) ;
      }
    }

    if (hasLabel)
    {
      { // send label
        // need to transform array of string (no fixed size for string) into array of array of char
        // to use connector to transfer
        // the strings must have fixed size which the maximum lenght over the string label.  
        int maxSize=0 ;
        for(int i=0; i<label.numElements();i++) 
          if (maxSize < label(i).size()) maxSize=label(i).size() ;
        MPI_Allreduce(MPI_IN_PLACE, &maxSize,1,MPI_INT,MPI_MAX, context->getIntraComm()) ;
        maxSize=maxSize+1 ;
        CArray<char,2> charArray(maxSize,label.numElements()) ;
        for(int j=0; j<label.numElements();j++) 
        {
          const char* str = label(j).c_str() ;
          int strSize=label(j).size()+1 ;
          for(int i=0; i<strSize; i++) charArray(i,j) = str[i] ;
        }
        CEventClient event(getType(), EVENT_ID_SEND_DISTRIBUTED_ATTRIBUTE);
        CMessage message ;
        message<<serverAxisId<<string("label")<<maxSize ;
        scattererConnector.transfer(maxSize, charArray, client, event,message) ;
      }
    }
  }

  void CAxis::recvDistributedAttributes(CEventServer& event)
  TRY
  {
    string axisId;
    string type ;
    for (auto& subEvent : event.subEvents) (*subEvent.buffer) >> axisId >> type ;
    get(axisId)->recvDistributedAttributes(event, type);
  }
  CATCH

  void CAxis::recvDistributedAttributes(CEventServer& event, const string& type)
  TRY
  {
    if (type=="value") 
    {
      gathererConnector_->transfer(event, value, 0.); 
    }
    else if (type=="bounds")
    {
      CArray<double,1> value ;
      gathererConnector_->transfer(event, 2, value, 0.); 
      bounds.resize(2,n) ;
      if (bounds.numElements() > 0 ) bounds=CArray<double,2>(value.dataFirst(),shape(2,n),neverDeleteData) ; 
    }
    else if (type=="label")
    {
      int maxSize ;
      for (auto& subEvent : event.subEvents) (*subEvent.buffer) >> maxSize ;
      CArray<char,1> value ;
      gathererConnector_->transfer(event, maxSize, value, '\0'); 
      CArray<char,2> charArray(maxSize,n) ;
      label.resize(n) ;
      if (n>0)
      {
        charArray=CArray<char,2>(value.dataFirst(),shape(maxSize,n),neverDeleteData) ;
        for(int j=0;j<n;j++)
        {
          int strSize ;
          for(int i=0;i<maxSize;i++) 
            if (charArray(i,j)=='\0') { strSize=i ; break; }
          string str(strSize,'\0') ;
          for(int i=0;i<strSize;i++) str[i]=charArray(i,j) ; 
          label(j)=str ;
        }
      } 
    }
  }
  CATCH

  DEFINE_REF_FUNC(Axis,axis)

   ///---------------------------------------------------------------

} // namespace xios
