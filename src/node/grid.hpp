#ifndef __XIOS_CGrid__
#define __XIOS_CGrid__

/// XIOS headers ///
#include "xios_spl.hpp"
#include "group_factory.hpp"

#include "declare_group.hpp"
#include "domain.hpp"
#include "axis.hpp"
#include "scalar.hpp"
#include "array_new.hpp"
#include "attribute_array.hpp"
#include "distribution_server.hpp"
#include "client_server_mapping.hpp"
#include "utils.hpp"
#include "transformation_enum.hpp"
#include "grid_local_connector.hpp"
#include "grid_elements.hpp"
#include "grid_scatterer_connector.hpp"
#include "grid_gatherer_connector.hpp"
#include "transformation_path.hpp"
#include "filter.hpp"
#include "grid_algorithm.hpp"


namespace xios {

   /// ////////////////////// Declarations ////////////////////// ///

   class CGridGroup;
   class CGridAttributes;
   class CDomainGroup;
   class CAxisGroup;
   class CScalarGroup;
   class CGrid;
   class CDistributionClient;
   class CDistributionServer;
   class CServerDistributionDescription;
   class CClientServerMapping;
   class CGridTransformation;

   ///--------------------------------------------------------------

   // Declare/Define CGridAttribute
   BEGIN_DECLARE_ATTRIBUTE_MAP(CGrid)
#  include "grid_attribute.conf"
   END_DECLARE_ATTRIBUTE_MAP(CGrid)

   ///--------------------------------------------------------------

   class CGrid
      : public CObjectTemplate<CGrid>
      , public CGridAttributes
   {
         /// typedef ///
         typedef CObjectTemplate<CGrid>   SuperClass;
         typedef CGridAttributes SuperClassAttribute;

      private:
        
        // define a structure to store elements (CDomain, CAxis, CScalar) using a void* and a type to cast the pointer
         enum EElementType { TYPE_SCALAR, TYPE_AXIS, TYPE_DOMAIN } ;
         struct SElement {void* ptr ; EElementType type ; CScalar* scalar ;  CAxis* axis ; CDomain* domain ; } ;
         vector<SElement> elements_ ;
         bool elementsComputed_ = false ; 
         /** retrieve the vector of elements as a structure containing a void* and the type of pointer */
         vector<SElement>& getElements(void) { if (!elementsComputed_) computeElements() ; return elements_ ; } 
         void computeElements(void) ;
         /** List order of axis and domain in a grid, if there is a domain, it will take value 2, axis 1, scalar 0 */
         std::vector<int> order_;

      public:

         typedef CGridAttributes RelAttributes;
         typedef CGridGroup      RelGroup;

         enum EEventId
         {
           EVENT_ID_ADD_DOMAIN, EVENT_ID_ADD_AXIS, EVENT_ID_ADD_SCALAR,
           EVENT_ID_SEND_MASK,

         };

         /// Constructeurs ///
         CGrid(void);
         explicit CGrid(const StdString& id);
         CGrid(const CGrid& grid);       // Not implemented yet.
         CGrid(const CGrid* const grid); // Not implemented yet.

         /// Traitements ///
//         void solveReference(void);

         void checkEligibilityForCompressedOutput();
         


         void checkMaskIndex(bool doCalculateIndex);

 //        virtual void toBinary  (StdOStream& os) const;
//         virtual void fromBinary(StdIStream& is);

         void addRelFileCompressed(const StdString& filename);

         /// Tests ///
         bool isCompressible(void) const;
         bool isWrittenCompressed(const StdString& filename) const;

      public:

         /// Accesseurs ///
         StdSize getDimension(void);

         StdSize  getDataSize(void) ;

         /**
          * Get the local data grid size, ie the size of the compressed grid (inside the workflow)
          * \return The size od the compressed grid
          */
         StdSize  getLocalDataSize(void) ;

 
         virtual void parse(xml::CXMLNode& node);

         /// Destructeur ///
         virtual ~CGrid(void);

      public:

         /// Accesseurs statiques ///
         static StdString GetName(void);
         static StdString GetDefName(void);

         static ENodeType GetType(void);

         /// Instanciateurs Statiques ///
         static CGrid* createGrid(CDomain* domain);
         static CGrid* createGrid(CDomain* domain, CAxis* axis);
         static CGrid* createGrid(const std::vector<CDomain*>& domains, const std::vector<CAxis*>& axis,
                                  const CArray<int,1>& axisDomainOrder = CArray<int,1>());
         static CGrid* createGrid(StdString id, const std::vector<CDomain*>& domains, const std::vector<CAxis*>& axis,
                                  const std::vector<CScalar*>& scalars, const CArray<int,1>& axisDomainOrder = CArray<int,1>());
         static CGrid* createGrid(const std::vector<CDomain*>& domains, const std::vector<CAxis*>& axis,
                                  const std::vector<CScalar*>& scalars, const CArray<int,1>& axisDomainOrder);
         static StdString generateId(const std::vector<CDomain*>& domains, const std::vector<CAxis*>& axis,
                                     const std::vector<CScalar*>& scalars, const CArray<int,1>& axisDomainOrder = CArray<int,1>());
         static StdString generateId(const CGrid* gridSrc, const CGrid* gridDest);
         static CGrid* cloneGrid(const StdString& idNewGrid, CGrid* gridSrc);

      public:            
         void solveDomainAxisRef(bool areAttributesChecked);
         void checkElementsAttributes(void) ;

         void solveDomainRef(bool checkAtt);
         void solveAxisRef(bool checkAtt);
         void solveScalarRef(bool checkAtt);
         void solveElementsRefInheritance(bool apply = true);
        // void solveTransformations();
         void solveDomainAxisBaseRef();

         CDomain* addDomain(const std::string& id=StdString());
         CAxis* addAxis(const std::string& id=StdString());
         CScalar* addScalar(const std::string& id=StdString());

      public:
         void sendGridToFileServer(CContextClient* client) ;
      private:
         std::set<CContextClient*> sendGridToFileServer_done_ ;
     
      public:
         void sendGridToCouplerOut(CContextClient* client, const string& fieldId) ;
      private:
         std::set<CContextClient*> sendGridToCouplerOut_done_ ;

      public:
         void makeAliasForCoupling(const string& fieldId) ;

      public:
         void sendAddDomain(const std::string& id,CContextClient* contextClient);
         void sendAddAxis(const std::string& id,CContextClient* contextClient);
         void sendAddScalar(const std::string& id,CContextClient* contextClient);
        
         static void recvAddDomain(CEventServer& event);
         void recvAddDomain(CBufferIn& buffer);
         static void recvAddAxis(CEventServer& event);
         void recvAddAxis(CBufferIn& buffer);
         static void recvAddScalar(CEventServer& event);
         void recvAddScalar(CBufferIn& buffer);

         static bool dispatchEvent(CEventServer& event);
       
       public:
         void setContextClient(CContextClient* contextClient);

         std::map<int, StdSize> getAttributesBufferSize(CContextClient* client, bool bufferForWriting = false);
         std::map<int, StdSize> getDataBufferSize(CContextClient* client, const std::string& id = "", bool bufferForWriting = false);
         std::vector<StdString> getDomainList();
         std::vector<StdString> getAxisList();
         std::vector<StdString> getScalarList();
         std::vector<CDomain*> getDomains();
         std::vector<CAxis*> getAxis();
         std::vector<CScalar*> getScalars();
         CDomain* getDomain(int domainIndex);
         CAxis* getAxis(int axisIndex);
         CScalar* getScalar(int scalarIndex);
         std::vector<int> getAxisOrder();
         std::vector<int> getGlobalDimension();
         bool isScalarGrid() const;         

         bool doGridHaveDataToWrite();
         bool doGridHaveDataDistributed(CContextClient* client = 0);

         ///////////////////////////////////////////
         ////////    TRANSFORMATIONS           /////
         ///////////////////////////////////////////
      public:
         CGridTransformation* getTransformations();

         std::vector<std::string> getAuxInputTransformGrid(void) ; 
         void setTransformationAlgorithms();
         pair<shared_ptr<CFilter>, shared_ptr<CFilter> > buildTransformationGraph(CGarbageCollector& gc, bool isSource, CGrid* gridSrc, double detectMissingValues,
                                                                                  double defaultValue, CGrid*& newGrid) ;
      private:
        CGridAlgorithm* gridAlgorithm_ = nullptr ;
      public:
        void setGridAlgorithm(CGridAlgorithm* gridAlgorithm) {gridAlgorithm_ = gridAlgorithm;}
        CGridAlgorithm* getGridAlgorithm(void) { return gridAlgorithm_ ;}
      private:
         bool isTransformed_;
         CGridTransformation* transformations_;
         bool hasTransform_;
       
        ///////////////////////////////////////////
      public:

        size_t getGlobalWrittenSize(void) ;
         
         bool isCompleted(void) ;
         void setCompleted(void) ;
         void unsetCompleted(void) ;


         bool hasMask(void) const;
        /** get mask pointer stored in mask_1d, or mask_2d, or..., or mask_7d */
         CArray<bool,1> mask_ ;
         CArray<bool,1>& getMask(void) ;

      private:
        /** Client-like distribution calculated based on the knowledge of the entire grid */
       CDistributionClient* clientDistribution_;
     public: 
       void computeClientDistribution(void) ;
     private:
       bool computeClientDistribution_done_ = false ;
     public:
       CDistributionClient* getClientDistribution(void); 

     private:   
        void setVirtualDomainGroup(CDomainGroup* newVDomainGroup);
        void setVirtualAxisGroup(CAxisGroup* newVAxisGroup);
        void setVirtualScalarGroup(CScalarGroup* newVScalarGroup);

        void setDomainList(const std::vector<CDomain*> domains = std::vector<CDomain*>());
        void setAxisList(const std::vector<CAxis*> axis = std::vector<CAxis*>());
        void setScalarList(const std::vector<CScalar*> scalars = std::vector<CScalar*>());

        CDomainGroup* getVirtualDomainGroup() const;
        CAxisGroup* getVirtualAxisGroup() const;
        CScalarGroup* getVirtualScalarGroup() const;

        int computeGridGlobalDimension(std::vector<int>& globalDim,
                                       const std::vector<CDomain*> domains,
                                       const std::vector<CAxis*> axis,
                                       const std::vector<CScalar*> scalars,
                                       const CArray<int,1>& axisDomainOrder);
        int getDistributedDimension();
      public:

        bool isDataDistributed(void) ; 
      private:

/** Clients that have to send a grid. There can be multiple clients in case of secondary server, otherwise only one client. */
        std::list<CContextClient*> clients;
        std::set<CContextClient*> clientsSet;

      private:
        bool isChecked;
        bool isDomainAxisChecked;
        bool isIndexSent;

        CDomainGroup* vDomainGroup_;
        CAxisGroup* vAxisGroup_;
        CScalarGroup* vScalarGroup_;
        std::vector<std::string> axisList_, domList_, scalarList_;
        bool isAxisListSet, isDomListSet, isScalarListSet;

/** Map storing local ranks of connected receivers. Key = size of receiver's intracomm.
  * It is calculated in computeConnectedClients(). */
        std::map<int, std::vector<int> > connectedServerRank_;

/** Map storing the size of data to be send. Key = size of receiver's intracomm
  * It is calculated in computeConnectedClients(). */
        std::map<int, std::map<int,size_t> > connectedDataSize_;

/** Ranks of connected receivers in case of reading. It is calculated in recvIndex(). */
        std::vector<int> connectedServerRankRead_;

/** Size of data to be send in case of reading. It is calculated in recvIndex(). */
        std::map<int,size_t> connectedDataSizeRead_;
      
         //! True if and only if the data defined on the grid can be outputted in a compressed way
        bool isCompressible_;
        std::set<std::string> relFilesCompressed;

        
        std::vector<int> axisPositionInGrid_;
        void computeAxisPositionInGrid(void) ;
        bool computeAxisPositionInGrid_done_ = false ;
        std::vector<int>& getAxisPositionInGrid(void) { if (!computeAxisPositionInGrid_done_) computeAxisPositionInGrid() ; return axisPositionInGrid_ ;}

        bool hasDomainAxisBaseRef_;        
        std::map<CGrid*, std::pair<bool,StdString> > gridSrc_;

     //////////////////////////////////////////////////////////////////////////////////////
     //  this part is related to distribution, element definition, views and connectors  //
     //////////////////////////////////////////////////////////////////////////////////////
      public:
       CGrid* duplicateSentGrid(void) ;
      private:
       static void recvMask(CEventServer& event) ;
       void receiveMask(CEventServer& event) ;

      private:  
        CGridLocalElements* gridLocalElements_= nullptr ;
        void computeGridLocalElements(void) ;
      public:
        CGridLocalElements* getGridLocalElements(void) { if (gridLocalElements_==nullptr) computeGridLocalElements() ; return gridLocalElements_ ;}

      private:
        CGridLocalConnector* modelToWorkflowConnector_ = nullptr ;
      public:
        void computeModelToWorkflowConnector(void) ;
        CGridLocalConnector* getModelToWorkflowConnector(void) { if (modelToWorkflowConnector_==nullptr) computeModelToWorkflowConnector() ; return modelToWorkflowConnector_;}

      private:
        CGridLocalConnector* workflowToModelConnector_ ;
      public:
        void computeWorkflowToModelConnector(void) ;
        CGridLocalConnector* getWorkflowToModelConnector(void) { if (workflowToModelConnector_==nullptr) computeWorkflowToModelConnector() ; return workflowToModelConnector_;}

      public: //? 
        void distributeGridToFileServer(CContextClient* client);
      
            
      private:
        CGridLocalConnector* workflowToFullConnector_ = nullptr;
      public:
        void computeWorkflowToFullConnector(void) ;
        CGridLocalConnector* getWorkflowToFullConnector(void) { if (workflowToFullConnector_==nullptr) computeWorkflowToFullConnector() ; return workflowToFullConnector_;}

      private:
        CGridLocalConnector* fullToWorkflowConnector_ = nullptr;
      public:
        void computeFullToWorkflowConnector(void) ;
        CGridLocalConnector* getFullToWorkflowConnector(void) { if (fullToWorkflowConnector_==nullptr) computeFullToWorkflowConnector() ; return fullToWorkflowConnector_;}

    

      private:
         CGridGathererConnector* clientFromClientConnector_ = nullptr ;
      public:
         CGridGathererConnector* getClientFromClientConnector(void) { if (clientFromClientConnector_==nullptr) computeClientFromClientConnector() ; return clientFromClientConnector_;}
         void computeClientFromClientConnector(void) ;

      private:
         map<CContextClient*, CGridScattererConnector*> clientToClientConnector_ ;
      public:
         CGridScattererConnector* getClientToClientConnector(CContextClient* client) { return clientToClientConnector_[client] ;} // make some test to see if connector exits for the given client
  

      private:
         map<CContextClient*,CGridGathererConnector*> clientFromServerConnector_  ;
      public:
         CGridGathererConnector* getClientFromServerConnector(CContextClient* client) { return clientFromServerConnector_[client];}
         void computeClientFromServerConnector(void) ;

      private:
         CGridScattererConnector* serverToClientConnector_=nullptr ;
      public:
         CGridScattererConnector* getServerToClientConnector(void) { if (serverToClientConnector_==nullptr) computeServerToClientConnector() ; return serverToClientConnector_;}
         void computeServerToClientConnector(void) ;
      private:
         map<CContextClient*, CGridScattererConnector*> clientToServerConnector_ ;
      public:
         CGridScattererConnector* getClientToServerConnector(CContextClient* client) { return clientToServerConnector_[client] ;} // make some test to see if connector exits for the given client
         
      private:
         CGridGathererConnector* serverFromClientConnector_ = nullptr ;
      public:
         CGridGathererConnector* getServerFromClientConnector(void) { if (serverFromClientConnector_==nullptr) computeServerFromClientConnector() ; return serverFromClientConnector_;}
         void computeServerFromClientConnector(void) ;

   }; // class CGrid

   // Declare/Define CGridGroup and CGridDefinition
   DECLARE_GROUP(CGrid);

   ///--------------------------------------------------------------

} // namespace xios

#endif // __XIOS_CGrid__
