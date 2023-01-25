#include "service_node.hpp"

namespace xios
{
 
  CServiceNode::CServiceNode(void) : CObjectTemplate<CServiceNode>(), CServiceNodeAttributes()
  { /* Ne rien faire de plus */ }

  CServiceNode::CServiceNode(const StdString & id) : CObjectTemplate<CServiceNode>(id), CServiceNodeAttributes()
  { /* Ne rien faire de plus */ }

  CServiceNode::~CServiceNode(void)
  { /* Ne rien faire de plus */ }


  void CServiceNode::parse(xml::CXMLNode & node)
  {
    SuperClass::parse(node);
  }

  void CServiceNode::allocateRessources(const string& poolId)
  {
    int nonEmpty=0 ;
    if (!nprocs.isEmpty()) nonEmpty++ ;
    if (!global_fraction.isEmpty()) nonEmpty++ ;
    if (!remain_fraction.isEmpty()) nonEmpty++ ;
    if (!remain.isEmpty()) nonEmpty++ ;
    if (nonEmpty==0) ERROR("void CServiceNode::allocateRessources(const string& poolId)",<<"A number a ressource for allocate service must be specified." 
                           <<"At least attributes, <nprocs> or <global_fraction> or <remain_fraction> or <remain> must be specified")
    else if (nonEmpty>1) ERROR("void void CServiceNode::allocateRessources(const string& poolId)",<<"Only one of these attributes : <nprocs> or <global_fraction>"
                               <<" or <remain_fraction> or <remain> must be specified to determine allocated ressources."
                               <<" More than one is currently specified")
    auto servicesManager=CXios::getServicesManager() ;
    auto ressourcesManager=CXios::getRessourcesManager() ;
    int nbRessources ;
    int globalRessources ;
    ressourcesManager->getPoolSize(poolId, globalRessources) ;
    int freeRessources ;
    ressourcesManager->getPoolFreeSize(poolId, freeRessources ) ;
    if (!nprocs.isEmpty()) nbRessources = nprocs ;
    if (!global_fraction.isEmpty()) nbRessources = std::round(globalRessources * global_fraction) ;
    if (!remain_fraction.isEmpty()) nbRessources = std::round(freeRessources * remain_fraction) ;
    if (!remain.isEmpty()) nbRessources = freeRessources ;
    if (nbRessources>freeRessources)
      ERROR("void CServiceNode::allocateRessources(const string& poolId)",<<"Cannot allocate required ressources for the service."
                                                       <<"  Required is : "<<nbRessources<<"  but free ressource is currently : "<<freeRessources)
    if (nb_partitions.isEmpty()) nb_partitions=1 ;
    if (nb_partitions > nbRessources) ERROR("void CServiceNode::allocateRessources(const string& poolId)",<<"Cannot allocate required ressources for the service."
                                                       <<"  The number of service partition : < nb_partitions = "<<nb_partitions<<" > "
                                                       <<"is greater than the required ressources required < nbRessources = "<<nbRessources<<" >")
    
    int serviceType ;
    if (type.isEmpty()) ERROR("void CServiceNode::allocateRessources(const string& poolId)",<<"Service type must be specified")
    if (type.getValue() == type_attr::writer) serviceType=CServicesManager::WRITER ;
    else if (type.getValue() == type_attr::reader) serviceType=CServicesManager::READER ;
    else if (type.getValue() == type_attr::gatherer) serviceType=CServicesManager::GATHERER ;

    string serviceId ;
    if (!name.isEmpty()) serviceId=name ;
    else if (!hasAutoGeneratedId() ) serviceId=getId() ;
    else ERROR("void CServiceNode::allocateRessources(const string& poolId)",<<"Service has no name or id, attributes <id> or <name> must be specified")
    servicesManager->createServices(poolId, serviceId, serviceType, nbRessources, nb_partitions, true) ;
  }

}