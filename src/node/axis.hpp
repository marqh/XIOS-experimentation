#ifndef __XIOS_CAxis__
#define __XIOS_CAxis__

/// XIOS headers ///
#include "xios_spl.hpp"
#include "group_factory.hpp"
#include "virtual_node.hpp"

#include "declare_group.hpp"
#include "declare_ref_func.hpp"
#include "declare_virtual_node.hpp"
#include "attribute_array.hpp"
#include "attribute_enum.hpp"
#include "attribute_enum_impl.hpp"
#include "server_distribution_description.hpp"
#include "transformation.hpp"
#include "transformation_enum.hpp"
#include "transformation_path.hpp"
#include "element.hpp"
#include "local_connector.hpp"
#include "scatterer_connector.hpp"
#include "gatherer_connector.hpp"
#include "distribution_type.hpp"
#include "generic_algorithm_transformation.hpp"


namespace xios
{
   /// ////////////////////// Déclarations ////////////////////// ///

   class CAxisGroup;
   class CAxisAttributes;
   class CAxis;

   ///--------------------------------------------------------------

   // Declare/Define CAxisAttribute
   BEGIN_DECLARE_ATTRIBUTE_MAP(CAxis)
#  include "axis_attribute.conf"
   END_DECLARE_ATTRIBUTE_MAP(CAxis)

   ///--------------------------------------------------------------

   class CAxis
      : public CObjectTemplate<CAxis>
      , public CAxisAttributes
   {
               /// typedef ///
         typedef CObjectTemplate<CAxis>   SuperClass;
         typedef CAxisAttributes SuperClassAttribute;
         
      public:
         enum EEventId
         {
           EVENT_ID_AXIS_DISTRIBUTION,
           EVENT_ID_SEND_DISTRIBUTED_ATTRIBUTE
         } ;

      public:
         typedef CAxisAttributes RelAttributes;
         typedef CAxisGroup      RelGroup;

      public:
         /// Constructeurs ///
         CAxis(void);
         explicit CAxis(const StdString & id);
         CAxis(const CAxis & axis);       // Not implemented yet.
         CAxis(const CAxis * const axis); // Not implemented yet.

         static CAxis* createAxis();

         /// Accesseurs ///
         const std::set<StdString> & getRelFiles(void) const;
 
         std::map<int, StdSize> getAttributesBufferSize(CContextClient* client, const std::vector<int>& globalDim, int orderPositionInGrid,
                                                        CServerDistributionDescription::ServerDistributionType disType = CServerDistributionDescription::BAND_DISTRIBUTION);

         /// Test ///
         bool IsWritten(const StdString & filename) const;
         bool isWrittenCompressed(const StdString& filename) const;
         bool isDistributed(void) const;
        
      public:
        /*!
            \brief return if the axis can be written or not in a compressed way.
            ie if there are some masked or indexed point on the domain. Valid only on server side.
            \return true if domain can be writtedn in a compressed way
         */ 
         bool isCompressible(void) { if (!isCompressibleComputed_) computeIsCompressible() ; return isCompressible_ ;} 
      private:
         bool isCompressible_ ; /** specify if the domain can be written in a compressed way */ 
         bool isCompressibleComputed_=false ; /** Indicate if compressability has been computed*/
         void computeIsCompressible() ;
        
      public:

         /// Mutateur ///
         void addRelFile(const StdString & filename);
         void addRelFileCompressed(const StdString& filename);

         

         /// Destructeur ///
         virtual ~CAxis(void);

         virtual void parse(xml::CXMLNode & node);

         void setContextClient(CContextClient* contextClient);

         /// Accesseurs statiques ///
         static StdString GetName(void);
         static StdString GetDefName(void);
         static ENodeType GetType(void);

         static bool dispatchEvent(CEventServer& event);         
        
         /// Vérifications ///
         void checkAttributes(void);
         bool checkAttributes_done_ = false ;

         size_t getGlobalWrittenSize(void) ;

      //////////////////////////
      ///// transformations ////
      //////////////////////////
      public:
        typedef CTransformation<CAxis>::TransformationMapTypes TransMapTypes;
      private:
        static bool initializeTransformationMap(std::map<StdString, ETranformationType>& m);
        static std::map<StdString, ETranformationType> transformationMapList_;
        static bool dummyTransformationMapList_;
        TransMapTypes transformationMap_;         

      public:
        CTransformation<CAxis>* addTransformation(ETranformationType transType, const StdString& id="");
        CTransformation<CAxis>* addTransformation(ETranformationType transType, CTransformation<CAxis>* transformation) ;
        void setTransformations(const TransMapTypes&);         
        void duplicateTransformation(CAxis*);
        TransMapTypes getAllTransformations();
        bool hasTransformation();
        void solveInheritanceTransformation_old(); // to remove later
        void solveInheritanceTransformation();
      private:
        bool solveInheritanceTransformation_done_= false ;
      private:
        CGenericAlgorithmTransformation* transformationAlgorithm_ = nullptr ;
      public:
        void setTransformationAlgorithm(CGenericAlgorithmTransformation* transformationAlgorithm) { transformationAlgorithm=transformationAlgorithm ;}
        CGenericAlgorithmTransformation* getTransformationAlgorithm(void) { return transformationAlgorithm_ ;}   
      private:
        CTransformationPaths transformationPaths_ ;
      public:
        CTransformationPaths getTransformationPaths(void) {return transformationPaths_;} 
        void setTransformationPaths(const CTransformationPaths& transformationPaths) { transformationPaths_=transformationPaths ;}

      ////////////////////////////
         bool isEqual(CAxis* axis);

      public: 
        bool hasValue;        
        bool hasBounds;
        bool hasLabel;

      private:
         void checkData();
         void checkMask();
         void checkBounds();
         void checkLabel();
      
      public:
         void sendAxisToCouplerOut(CContextClient* client, const std::vector<int>& globalDim, int orderPositionInGrid, const string& fieldId, int posInGrid) ;
      private:
         std::set<CContextClient*> sendAxisToCouplerOut_done_ ;
    
      public:
         void makeAliasForCoupling(const string& fieldId, int posInGrid) ;


      private:

/** Clients that have to send a axis. There can be multiple clients in case of secondary server, otherwise only one client. */
         std::list<CContextClient*> clients;
         std::set<CContextClient*> clientsSet;

      private:
         /** define if the axis is completed or not ie all attributes have been received before in case 
             of grid reading from file or coupling */ 
         bool isCompleted_=true ;  
      public:     
         /*!
           \brief Check if a axis is completed
           Before make any axis processing, we must be sure that all axis informations have
           been sent, for exemple when reading a grid in a file or when grid elements are sent by an
           other context (coupling). So all direct reference of the axis (axis_ref) must be also completed
           \return true if axis and axis reference are completed
          */
         bool isCompleted(void)
         {
           if (hasDirectAxisReference()) if (!getDirectAxisReference()->isCompleted()) return false;
           else return isCompleted_ ;
         }
         void setCompleted(void) { isCompleted_=true ; }
         void unsetCompleted(void) { isCompleted_=false ; }
      
      private:
         bool isChecked;
         std::set<StdString> relFiles, relFilesCompressed;
          std::map<int, std::unordered_map<int, vector<size_t> > > indSrv_; // Global index of each client sent to server
         std::map<int, std::vector<int> > connectedServerRank_;

       //////////////////////////////////////////////////////////////////////////////////////
       //  this part is related to distribution, element definition, views and connectors  //
       //////////////////////////////////////////////////////////////////////////////////////
         
        private:
         CLocalElement* localElement_ = nullptr ;
         void initializeLocalElement(void) ;
        public: 
         CLocalElement* getLocalElement(void) { if (localElement_==nullptr) initializeLocalElement() ; return localElement_ ; }
         CLocalView* getLocalView(CElementView::type type) { return getLocalElement()->getView(type) ;}
        private:
         void addFullView(void) ;
         void addWorkflowView(void) ;
         void addModelView(void) ;

        private:
         CLocalConnector* modelToWorkflowConnector_ ;
         void computeModelToWorkflowConnector(void)  ;
        public:
         CLocalConnector* getModelToWorkflowConnector(void) { if (modelToWorkflowConnector_==nullptr) computeModelToWorkflowConnector() ; return modelToWorkflowConnector_ ;}
       
       public:
         void computeRemoteElement(CContextClient* client, EDistributionType) ;
         void distributeToServer(CContextClient* client, std::map<int, CArray<size_t,1>>& globalIndex, CScattererConnector* &scattererConnector,
                                 const string& axisId="") ;

         static void recvAxisDistribution(CEventServer& event) ;
         void receivedAxisDistribution(CEventServer& event, int phasis) ;
         void setServerMask(CArray<bool,1>& serverMask, CContextClient* client ) ;
         void sendDistributedAttributes(CContextClient* client, CScattererConnector& scattererConnector, const string& axisId) ;
         static void recvDistributedAttributes(CEventServer& event) ;
         void recvDistributedAttributes(CEventServer& event, const string& type) ;
       private:
         map<CContextClient*, CDistributedElement*> remoteElement_ ;
       public: 
         CDistributedElement* getRemoteElement(CContextClient* client) {return remoteElement_[client] ;}
       private:
         map<CContextClient*, CScattererConnector*> clientToServerConnector_ ;
       public: 
         CScattererConnector* getClientToServerConnector(CContextClient* client) { return clientToServerConnector_[client] ;}
       private:
         CGathererConnector*  gathererConnector_ ;
       public:
         CGathererConnector* getGathererConnector(void) { return gathererConnector_ ;}
       private:
         CGathererConnector* serverFromClientConnector_ ;
         CDistributedElement* elementFrom_ ;
       public:
        CGathererConnector* getServerFromClientConnector(void) { return serverFromClientConnector_ ;}

       private:
         CScattererConnector*  serverToClientConnector_ = nullptr ;
       public: 
         CScattererConnector* getServerToClientConnector(void) { return serverToClientConnector_ ;} 

       private:
          map<CContextClient*,CGathererConnector*>  clientFromServerConnector_  ;
       public: 
         CGathererConnector* getClientFromServerConnector(CContextClient* client) { return clientFromServerConnector_[client] ;} 

         DECLARE_REF_FUNC(Axis,axis)
   }; // class CAxis

   ///--------------------------------------------------------------

   // Declare/Define CAxisGroup and CAxisDefinition
   DECLARE_GROUP(CAxis);
} // namespace xios

#endif // __XIOS_CAxis__
