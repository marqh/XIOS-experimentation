#include "context.hpp"
#include "attribute_template.hpp"
#include "object_template.hpp"
#include "group_template.hpp"

#include "calendar_type.hpp"
#include "duration.hpp"

#include "context_client.hpp"
#include "context_server.hpp"
#include "nc4_data_output.hpp"
#include "node_type.hpp"
#include "message.hpp"
#include "type.hpp"
#include "xmlioserver_spl.hpp"

namespace xios {

  shared_ptr<CContextGroup> CContext::root;

   /// ////////////////////// Définitions ////////////////////// ///

   CContext::CContext(void)
      : CObjectTemplate<CContext>(), CContextAttributes()
      , calendar(),hasClient(false),hasServer(false), isPostProcessed(false), dataSize_(), idServer_()
   { /* Ne rien faire de plus */ }

   CContext::CContext(const StdString & id)
      : CObjectTemplate<CContext>(id), CContextAttributes()
      , calendar(),hasClient(false),hasServer(false), isPostProcessed(false), dataSize_(), idServer_()
   { /* Ne rien faire de plus */ }

   CContext::~CContext(void)
   {
     if (hasClient) delete client;
     if (hasServer) delete server;
   }

   //----------------------------------------------------------------
   //! Get name of context
   StdString CContext::GetName(void)   { return (StdString("context")); }
   StdString CContext::GetDefName(void){ return (CContext::GetName()); }
   ENodeType CContext::GetType(void)   { return (eContext); }

   //----------------------------------------------------------------
   /*!
   \brief Get context group (context root)
   \return Context root
   */
   CContextGroup* CContext::getRoot(void)
   {
      if (root.get()==NULL) root=shared_ptr<CContextGroup>(new CContextGroup(xml::CXMLNode::GetRootName()));
      return root.get();
   }


   //----------------------------------------------------------------
   /*!
   \brief Get calendar of a context
   \return Calendar
   */
   boost::shared_ptr<CCalendar> CContext::getCalendar(void) const
   {
      return (this->calendar);
   }

   //----------------------------------------------------------------
   /*!
   \brief Set a context with a calendar
   \param[in] newCalendar new calendar
   */
   void CContext::setCalendar(boost::shared_ptr<CCalendar> newCalendar)
   {
      this->calendar = newCalendar;
   }

   //----------------------------------------------------------------
   /*!
   \brief Parse xml file and write information into context object
   \param [in] node xmld node corresponding in xml file
   */
   void CContext::parse(xml::CXMLNode & node)
   {
      CContext::SuperClass::parse(node);

      // PARSING POUR GESTION DES ENFANTS
      xml::THashAttributes attributes = node.getAttributes();

      if (attributes.end() != attributes.find("src"))
      {
         StdIFStream ifs ( attributes["src"].c_str() , StdIFStream::in );
         if ( (ifs.rdstate() & std::ifstream::failbit ) != 0 )
            ERROR("void CContext::parse(xml::CXMLNode & node)",
                  <<endl<< "Can not open <"<<attributes["src"].c_str()<<"> file" );
         if (!ifs.good())
            ERROR("CContext::parse(xml::CXMLNode & node)",
                  << "[ filename = " << attributes["src"] << " ] Bad xml stream !");
         xml::CXMLParser::ParseInclude(ifs, attributes["src"], *this);
      }

      if (node.getElementName().compare(CContext::GetName()))
         DEBUG("Le noeud is wrong defined but will be considered as a context !");

      if (!(node.goToChildElement()))
      {
         DEBUG("Le context ne contient pas d'enfant !");
      }
      else
      {
         do { // Parcours des contextes pour traitement.

            StdString name = node.getElementName();
            attributes.clear();
            attributes = node.getAttributes();

            if (attributes.end() != attributes.find("id"))
            {
              DEBUG(<< "Definition node has an id,"
                    << "it will not be taking account !");
            }

#define DECLARE_NODE(Name_, name_)    \
   if (name.compare(C##Name_##Definition::GetDefName()) == 0) \
   { C##Name_##Definition::create(C##Name_##Definition::GetDefName()) -> parse(node); continue; }
#define DECLARE_NODE_PAR(Name_, name_)
#include "node_type.conf"

            DEBUG(<< "The element \'"     << name
                  << "\' in the context \'" << CContext::getCurrent()->getId()
                  << "\' is not a definition !");

         } while (node.goToNextElement());

         node.goToParentElement(); // Retour au parent
      }
   }

   //----------------------------------------------------------------
   //! Show tree structure of context
   void CContext::ShowTree(StdOStream & out)
   {
      StdString currentContextId = CContext::getCurrent() -> getId();
      std::vector<CContext*> def_vector =
         CContext::getRoot()->getChildList();
      std::vector<CContext*>::iterator
         it = def_vector.begin(), end = def_vector.end();

      out << "<? xml version=\"1.0\" ?>" << std::endl;
      out << "<"  << xml::CXMLNode::GetRootName() << " >" << std::endl;

      for (; it != end; it++)
      {
         CContext* context = *it;
         CContext::setCurrent(context->getId());
         out << *context << std::endl;
      }

      out << "</" << xml::CXMLNode::GetRootName() << " >" << std::endl;
      CContext::setCurrent(currentContextId);
   }


   //----------------------------------------------------------------

   //! Convert context object into string (to print)
   StdString CContext::toString(void) const
   {
      StdOStringStream oss;
      oss << "<" << CContext::GetName()
          << " id=\"" << this->getId() << "\" "
          << SuperClassAttribute::toString() << ">" << std::endl;
      if (!this->hasChild())
      {
         //oss << "<!-- No definition -->" << std::endl; // fait planter l'incrémentation
      }
      else
      {

#define DECLARE_NODE(Name_, name_)    \
   if (C##Name_##Definition::has(C##Name_##Definition::GetDefName())) \
   oss << * C##Name_##Definition::get(C##Name_##Definition::GetDefName()) << std::endl;
#define DECLARE_NODE_PAR(Name_, name_)
#include "node_type.conf"

      }

      oss << "</" << CContext::GetName() << " >";

      return (oss.str());
   }

   //----------------------------------------------------------------

   /*!
   \brief Find all inheritace among objects in a context.
   \param [in] apply (true) write attributes of parent into ones of child if they are empty
                     (false) write attributes of parent into a new container of child
   \param [in] parent unused
   */
   void CContext::solveDescInheritance(bool apply, const CAttributeMap * const UNUSED(parent))
   {
#define DECLARE_NODE(Name_, name_)    \
   if (C##Name_##Definition::has(C##Name_##Definition::GetDefName())) \
     C##Name_##Definition::get(C##Name_##Definition::GetDefName())->solveDescInheritance(apply);
#define DECLARE_NODE_PAR(Name_, name_)
#include "node_type.conf"
   }

   //----------------------------------------------------------------

   //! Verify if all root definition in the context have child.
   bool CContext::hasChild(void) const
   {
      return (
#define DECLARE_NODE(Name_, name_)    \
   C##Name_##Definition::has(C##Name_##Definition::GetDefName())   ||
#define DECLARE_NODE_PAR(Name_, name_)
#include "node_type.conf"
      false);
}

//   //----------------------------------------------------------------
//
//   void CContext::solveFieldRefInheritance(bool apply)
//   {
//      if (!this->hasId()) return;
//      vector<CField*> allField = CField::getAll();
////              = CObjectTemplate<CField>::GetAllVectobject(this->getId());
//      std::vector<CField*>::iterator
//         it = allField.begin(), end = allField.end();
//
//      for (; it != end; it++)
//      {
//         CField* field = *it;
//         field->solveRefInheritance(apply);
//      }
//   }

   //----------------------------------------------------------------

   void CContext::CleanTree(void)
   {
#define DECLARE_NODE(Name_, name_) C##Name_##Definition::ClearAllAttributes();
#define DECLARE_NODE_PAR(Name_, name_)
#include "node_type.conf"
   }
   ///---------------------------------------------------------------

   //! Initialize client side
   void CContext::initClient(MPI_Comm intraComm, MPI_Comm interComm, CContext* cxtServer)
   {
     hasClient=true;
     client = new CContextClient(this,intraComm, interComm, cxtServer);
   }

   void CContext::setClientServerBuffer()
   {
     if (hasClient)
     {
       client->setBufferSize(getDataSize());
     }
   }

   //! Verify whether a context is initialized
   bool CContext::isInitialized(void)
   {
     return hasClient;
   }

   //! Initialize server
   void CContext::initServer(MPI_Comm intraComm,MPI_Comm interComm)
   {
     hasServer=true;
     server = new CContextServer(this,intraComm,interComm);
   }

   //! Server side: Put server into a loop in order to listen message from client
   bool CContext::eventLoop(void)
   {
     return server->eventLoop();
   }

   //! Terminate a context
   void CContext::finalize(void)
   {
      if (hasClient && !hasServer)
      {
         client->finalize();
      }
      if (hasServer)
      {
        closeAllFile();
      }
   }

   /*!
   \brief Close all the context defintion and do processing data
      After everything is well defined on client side, they will be processed and sent to server
   From the version 2.0, sever and client work no more on the same database. Moreover, client(s) will send
   all necessary information to server, from which each server can build its own database.
   Because the role of server is to write out field data on a specific netcdf file,
   the only information that it needs is the enabled files
   and the active fields (fields will be written onto active files)
   */
   void CContext::closeDefinition(void)
   {

     if (hasClient)
     {
       // After xml is parsed, there are some more works with post processing
       postProcessing();
//
       setClientServerBuffer();
     }

     if (hasClient && !hasServer)
     {
      // Send all attributes of current context to server
      this->sendAllAttributesToServer();

      // Send all attributes of current calendar
      CCalendarWrapper::get(CCalendarWrapper::GetDefName())->sendAllAttributesToServer();

      // We have enough information to send to server
      // First of all, send all enabled files
       sendEnabledFiles();

      // Then, send all enabled fields
       sendEnabledFields();

      // At last, we have all info of domain and axis, then send them
//       sendRefDomainsAxis();

      // After that, send all grid (if any)
       sendRefGrid();
    }

    // Now tell server that it can process all messages from client
    if (hasClient && !hasServer) this->sendCloseDefinition();

    // We have a xml tree on the server side and now, it should be also processed
    if (hasClient && !hasServer) sendPostProcessing();

    // There are some processings that should be done after all of above. For example: check mask or index
//    if (hasClient && !hasServer)
    if (hasClient)
    {
      this->solveAllRefOfEnabledFields(true);
      this->buildAllExpressionOfEnabledFields();
    }



//      if (hasClient)
//      {
//        //solveCalendar();
//
//        // Résolution des héritages pour le context actuel.
////        this->solveAllInheritance();
//
//
////        //Initialisation du vecteur 'enabledFiles' contenant la liste des fichiers à sortir.
////        this->findEnabledFiles();
//
//        this->processEnabledFiles();
//
//        this->solveAllGridRef();
//      }




//      solveCalendar();
//
//      // Résolution des héritages pour le context actuel.
//      this->solveAllInheritance();
//
//      //Initialisation du vecteur 'enabledFiles' contenant la liste des fichiers à sortir.
//      this->findEnabledFiles();
//
//
//      this->processEnabledFiles();

/*
      //Recherche des champs à sortir (enable à true + niveau de sortie correct)
      // pour chaque fichier précédemment listé.
      this->findAllEnabledFields();

      // Résolution des références de grilles pour chacun des champs.
      this->solveAllGridRef();

      // Traitement des opérations.
      this->solveAllOperation();

      // Traitement des expressions.
      this->solveAllExpression();
*/
      // Nettoyage de l'arborescence
      if (hasClient && !hasServer) CleanTree(); // Only on client side??
//      if (hasClient) CleanTree();
      if (hasClient) sendCreateFileHeader();
   }

   void CContext::findAllEnabledFields(void)
   {
     for (unsigned int i = 0; i < this->enabledFiles.size(); i++)
     (void)this->enabledFiles[i]->getEnabledFields();
   }

   void CContext::solveAllRefOfEnabledFields(bool sendToServer)
   {
     int size = this->enabledFiles.size();
     for (int i = 0; i < size; ++i)
     {
       this->enabledFiles[i]->solveAllRefOfEnabledFields(sendToServer);
     }
   }

   void CContext::buildAllExpressionOfEnabledFields()
   {
     int size = this->enabledFiles.size();
     for (int i = 0; i < size; ++i)
     {
       this->enabledFiles[i]->buildAllExpressionOfEnabledFields();
     }
   }

   void CContext::solveAllInheritance(bool apply)
   {
     // Résolution des héritages descendants (càd des héritages de groupes)
     // pour chacun des contextes.
      solveDescInheritance(apply);

     // Résolution des héritages par référence au niveau des fichiers.
      const vector<CFile*> allFiles=CFile::getAll();
      const vector<CGrid*> allGrids= CGrid::getAll();

     //if (hasClient && !hasServer)
      if (hasClient)
      {
        for (unsigned int i = 0; i < allFiles.size(); i++)
          allFiles[i]->solveFieldRefInheritance(apply);
      }

      unsigned int vecSize = allGrids.size();
      unsigned int i = 0;
      for (i = 0; i < vecSize; ++i)
        allGrids[i]->solveDomainAxisRefInheritance(apply);

   }

   void CContext::findEnabledFiles(void)
   {
      const std::vector<CFile*> allFiles = CFile::getAll();

      for (unsigned int i = 0; i < allFiles.size(); i++)
         if (!allFiles[i]->enabled.isEmpty()) // Si l'attribut 'enabled' est défini.
         {
            if (allFiles[i]->enabled.getValue()) // Si l'attribut 'enabled' est fixé à vrai.
               enabledFiles.push_back(allFiles[i]);
         }
         else enabledFiles.push_back(allFiles[i]); // otherwise true by default


      if (enabledFiles.size() == 0)
         DEBUG(<<"Aucun fichier ne va être sorti dans le contexte nommé \""
               << getId() << "\" !");
   }

   void CContext::closeAllFile(void)
   {
     std::vector<CFile*>::const_iterator
            it = this->enabledFiles.begin(), end = this->enabledFiles.end();

     for (; it != end; it++)
     {
       info(30)<<"Closing File : "<<(*it)->getId()<<endl;
       (*it)->close();
     }
   }

   /*!
   \brief Dispatch event received from client
      Whenever a message is received in buffer of server, it will be processed depending on
   its event type. A new event type should be added in the switch list to make sure
   it processed on server side.
   \param [in] event: Received message
   */
   bool CContext::dispatchEvent(CEventServer& event)
   {

      if (SuperClass::dispatchEvent(event)) return true;
      else
      {
        switch(event.type)
        {
           case EVENT_ID_CLOSE_DEFINITION :
             recvCloseDefinition(event);
             return true;
             break;
           case EVENT_ID_UPDATE_CALENDAR :
             recvUpdateCalendar(event);
             return true;
             break;
           case EVENT_ID_CREATE_FILE_HEADER :
             recvCreateFileHeader(event);
             return true;
             break;
           case EVENT_ID_POST_PROCESS:
             recvPostProcessing(event);
             return true;
             break;
           default :
             ERROR("bool CContext::dispatchEvent(CEventServer& event)",
                    <<"Unknown Event");
           return false;
         }
      }
   }

   //! Client side: Send a message to server to make it close
   void CContext::sendCloseDefinition(void)
   {
     CEventClient event(getType(),EVENT_ID_CLOSE_DEFINITION);
     if (client->isServerLeader())
     {
       CMessage msg;
       msg<<this->getIdServer();
       event.push(client->getServerLeader(),1,msg);
       client->sendEvent(event);
     }
     else client->sendEvent(event);
   }

   //! Server side: Receive a message of client announcing a context close
   void CContext::recvCloseDefinition(CEventServer& event)
   {

      CBufferIn* buffer=event.subEvents.begin()->buffer;
      string id;
      *buffer>>id;
      get(id)->closeDefinition();
   }

   //! Client side: Send a message to update calendar in each time step
   void CContext::sendUpdateCalendar(int step)
   {
     if (!hasServer)
     {
       CEventClient event(getType(),EVENT_ID_UPDATE_CALENDAR);
       if (client->isServerLeader())
       {
         CMessage msg;
         msg<<this->getIdServer()<<step;
         event.push(client->getServerLeader(),1,msg);
         client->sendEvent(event);
       }
       else client->sendEvent(event);
     }
   }

   //! Server side: Receive a message of client annoucing calendar update
   void CContext::recvUpdateCalendar(CEventServer& event)
   {

      CBufferIn* buffer=event.subEvents.begin()->buffer;
      string id;
      *buffer>>id;
      get(id)->recvUpdateCalendar(*buffer);
   }

   //! Server side: Receive a message of client annoucing calendar update
   void CContext::recvUpdateCalendar(CBufferIn& buffer)
   {
      int step;
      buffer>>step;
      updateCalendar(step);
   }

   //! Client side: Send a message to create header part of netcdf file
   void CContext::sendCreateFileHeader(void)
   {
     CEventClient event(getType(),EVENT_ID_CREATE_FILE_HEADER);
     if (client->isServerLeader())
     {
       CMessage msg;
       msg<<this->getIdServer();
       event.push(client->getServerLeader(),1,msg);
       client->sendEvent(event);
     }
     else client->sendEvent(event);
   }

   //! Server side: Receive a message of client annoucing the creation of header part of netcdf file
   void CContext::recvCreateFileHeader(CEventServer& event)
   {
      CBufferIn* buffer=event.subEvents.begin()->buffer;
      string id;
      *buffer>>id;
      get(id)->recvCreateFileHeader(*buffer);
   }

   //! Server side: Receive a message of client annoucing the creation of header part of netcdf file
   void CContext::recvCreateFileHeader(CBufferIn& buffer)
   {
      createFileHeader();
   }

   //! Client side: Send a message to do some post processing on server
   void CContext::sendPostProcessing()
   {
     if (!hasServer)
     {
       CEventClient event(getType(),EVENT_ID_POST_PROCESS);
       if (client->isServerLeader())
       {
         CMessage msg;
         msg<<this->getIdServer();
         event.push(client->getServerLeader(),1,msg);
         client->sendEvent(event);
       }
       else client->sendEvent(event);
     }
   }

   //! Server side: Receive a message to do some post processing
   void CContext::recvPostProcessing(CEventServer& event)
   {
      CBufferIn* buffer=event.subEvents.begin()->buffer;
      string id;
      *buffer>>id;
      get(id)->recvPostProcessing(*buffer);
   }

   //! Server side: Receive a message to do some post processing
   void CContext::recvPostProcessing(CBufferIn& buffer)
   {
      CCalendarWrapper::get(CCalendarWrapper::GetDefName())->createCalendar();
      postProcessing();
   }

   const StdString& CContext::getIdServer()
   {
      if (hasClient)
      {
        idServer_ = this->getId();
        idServer_ += "_server";
        return idServer_;
      }
      if (hasServer) return (this->getId());
   }

   /*!
   \brief Do some simple post processings after parsing xml file
      After the xml file (iodef.xml) is parsed, it is necessary to build all relations among
   created object, e.g: inhertance among fields, domain, axis. After that, all fiels as well as their parents (reference fields),
   which will be written out into netcdf files, are processed
   */
   void CContext::postProcessing()
   {
     if (isPostProcessed) return;

      // Make sure the calendar was correctly created
      if (!calendar)
        ERROR("CContext::postProcessing()", << "A calendar must be defined for the context \"" << getId() << "!\"")
      else if (calendar->getTimeStep() == NoneDu)
        ERROR("CContext::postProcessing()", << "A timestep must be defined for the context \"" << getId() << "!\"")
      // Calendar first update to set the current date equals to the start date
      calendar->update(0);

      // Find all inheritance in xml structure
      this->solveAllInheritance();

      //Initialisation du vecteur 'enabledFiles' contenant la liste des fichiers à sortir.
      this->findEnabledFiles();

      // Find all enabled fields of each file
      this->findAllEnabledFields();

      // Search and rebuild all reference object of enabled fields
      this->solveAllRefOfEnabledFields(false);
      isPostProcessed = true;
   }

   std::map<int, StdSize>& CContext::getDataSize()
   {
     // Set of grid used by enabled fields
     std::set<StdString> usedGrid;

     // Find all reference domain and axis of all active fields
     int numEnabledFiles = this->enabledFiles.size();
     for (int i = 0; i < numEnabledFiles; ++i)
     {
       std::vector<CField*> enabledFields = this->enabledFiles[i]->getEnabledFields();
       int numEnabledFields = enabledFields.size();
       for (int j = 0; j < numEnabledFields; ++j)
       {
//         const std::pair<StdString, StdString>& prDomAxisId = enabledFields[j]->getDomainAxisIds();
         StdString currentGrid = enabledFields[j]->grid->getId();
         const std::map<int, StdSize> mapSize = enabledFields[j]->getGridDataSize();
         if (dataSize_.empty())
         {
           dataSize_ = mapSize;
           usedGrid.insert(currentGrid);
//           domainIds.insert(prDomAxisId.first);
         }
         else
         {
           std::map<int, StdSize>::const_iterator it = mapSize.begin(), itE = mapSize.end();
           if (usedGrid.find(currentGrid) == usedGrid.end())
           {
             for (; it != itE; ++it)
             {
               if (0 < dataSize_.count(it->first)) dataSize_[it->first] += it->second;
               else dataSize_.insert(make_pair(it->first, it->second));
             }
           } else
           {
             for (; it != itE; ++it)
             {
               if (0 < dataSize_.count(it->first))
                if (CXios::isOptPerformance) dataSize_[it->first] += it->second;
                else
                {
                  if (dataSize_[it->first] < it->second) dataSize_[it->first] = it->second;
                }
               else dataSize_.insert(make_pair(it->first, it->second));
             }
           }
         }
       }
     }

     return dataSize_;
   }

   //! Client side: Send infomation of active files (files are enabled to write out)
   void CContext::sendEnabledFiles()
   {
     int size = this->enabledFiles.size();

     // In a context, each type has a root definition, e.g: axis, domain, field.
     // Every object must be a child of one of these root definition. In this case
     // all new file objects created on server must be children of the root "file_definition"
     StdString fileDefRoot("file_definition");
     CFileGroup* cfgrpPtr = CFileGroup::get(fileDefRoot);

     for (int i = 0; i < size; ++i)
     {
       cfgrpPtr->sendCreateChild(this->enabledFiles[i]->getId());
       this->enabledFiles[i]->sendAllAttributesToServer();
       this->enabledFiles[i]->sendAddAllVariables();
     }
   }

   //! Client side: Send information of active fields (ones are written onto files)
   void CContext::sendEnabledFields()
   {
     int size = this->enabledFiles.size();
     for (int i = 0; i < size; ++i)
     {
       this->enabledFiles[i]->sendEnabledFields();
     }
   }

   //! Client side: Send information of reference grid of active fields
   void CContext::sendRefGrid()
   {
     std::set<StdString> gridIds;
     int sizeFile = this->enabledFiles.size();
     CFile* filePtr(NULL);

     // Firstly, find all reference grids of all active fields
     for (int i = 0; i < sizeFile; ++i)
     {
       filePtr = this->enabledFiles[i];
       std::vector<CField*> enabledFields = filePtr->getEnabledFields();
       int sizeField = enabledFields.size();
       for (int numField = 0; numField < sizeField; ++numField)
       {
         if (0 != enabledFields[numField]->getRelGrid())
           gridIds.insert(CGrid::get(enabledFields[numField]->getRelGrid())->getId());
       }
     }

     // Create all reference grids on server side
     StdString gridDefRoot("grid_definition");
     CGridGroup* gridPtr = CGridGroup::get(gridDefRoot);
     std::set<StdString>::const_iterator it, itE = gridIds.end();
     for (it = gridIds.begin(); it != itE; ++it)
     {
       gridPtr->sendCreateChild(*it);
       CGrid::get(*it)->sendAllAttributesToServer();
       CGrid::get(*it)->sendAllDomains();
       CGrid::get(*it)->sendAllAxis();
     }
   }


   //! Client side: Send information of reference domain and axis of active fields
//   void CContext::sendRefDomainsAxis()
//   {
//     std::set<StdString> domainIds;
//     std::set<StdString> axisIds;
//
//     // Find all reference domain and axis of all active fields
//     int numEnabledFiles = this->enabledFiles.size();
//     for (int i = 0; i < numEnabledFiles; ++i)
//     {
//       std::vector<CField*> enabledFields = this->enabledFiles[i]->getEnabledFields();
//       int numEnabledFields = enabledFields.size();
//       for (int j = 0; j < numEnabledFields; ++j)
//       {
//         const std::pair<StdString, StdString>& prDomAxisId = enabledFields[j]->getDomainAxisIds();
//         domainIds.insert(prDomAxisId.first);
//         axisIds.insert(prDomAxisId.second);
//       }
//     }
//
//     // Create all reference axis on server side
//     std::set<StdString>::iterator itDom, itAxis;
//     std::set<StdString>::const_iterator itE;
//
//     StdString axiDefRoot("axis_definition");
//     CAxisGroup* axisPtr = CAxisGroup::get(axiDefRoot);
//     itE = axisIds.end();
//     for (itAxis = axisIds.begin(); itAxis != itE; ++itAxis)
//     {
//       if (!itAxis->empty())
//       {
//         axisPtr->sendCreateChild(*itAxis);
//         CAxis::get(*itAxis)->sendAllAttributesToServer();
//       }
//     }
//
//     // Create all reference domains on server side
//     StdString domDefRoot("domain_definition");
//     CDomainGroup* domPtr = CDomainGroup::get(domDefRoot);
//     itE = domainIds.end();
//     for (itDom = domainIds.begin(); itDom != itE; ++itDom)
//     {
//       if (!itDom->empty()) {
//          domPtr->sendCreateChild(*itDom);
//          CDomain::get(*itDom)->sendAllAttributesToServer();
//       }
//     }
//   }

   //! Update calendar in each time step
   void CContext::updateCalendar(int step)
   {
      info(50)<<"updateCalendar : before : "<<calendar->getCurrentDate()<<endl;
      calendar->update(step);
      info(50)<<"updateCalendar : after : "<<calendar->getCurrentDate()<<endl;
   }

   //! Server side: Create header of netcdf file
   void CContext::createFileHeader(void )
   {
      vector<CFile*>::const_iterator it;

      for (it=enabledFiles.begin(); it != enabledFiles.end(); it++)
      {
         (*it)->initFile();
      }
   }

   //! Get current context
   CContext* CContext::getCurrent(void)
   {
     return CObjectFactory::GetObject<CContext>(CObjectFactory::GetCurrentContextId()).get();
   }

   /*!
   \brief Set context with an id be the current context
   \param [in] id identity of context to be set to current
   */
   void CContext::setCurrent(const string& id)
   {
     CObjectFactory::SetCurrentContextId(id);
     CGroupFactory::SetCurrentContextId(id);
   }

  /*!
  \brief Create a context with specific id
  \param [in] id identity of new context
  \return pointer to the new context or already-existed one with identity id
  */
  CContext* CContext::create(const StdString& id)
  {
    CContext::setCurrent(id);

    bool hasctxt = CContext::has(id);
    CContext* context = CObjectFactory::CreateObject<CContext>(id).get();
    getRoot();
    if (!hasctxt) CGroupFactory::AddChild(root, context->getShared());

#define DECLARE_NODE(Name_, name_) \
    C##Name_##Definition::create(C##Name_##Definition::GetDefName());
#define DECLARE_NODE_PAR(Name_, name_)
#include "node_type.conf"

    return (context);
  }
} // namespace xios
