#include "domain.hpp"

#include "attribute_template.hpp"
#include "object_template.hpp"
#include "group_template.hpp"

#include "xios_spl.hpp"
#include "event_client.hpp"
#include "event_server.hpp"
#include "buffer_in.hpp"
#include "message.hpp"
#include "type.hpp"
#include "context.hpp"
#include "context_client.hpp"
#include "array_new.hpp"
#include "server_distribution_description.hpp"
#include "client_server_mapping_distributed.hpp"
#include "zoom_domain.hpp"
#include "interpolate_from_file_domain.hpp"

namespace xios {

   /// ////////////////////// Définitions ////////////////////// ///

   CDomain::CDomain(void)
      : CObjectTemplate<CDomain>(), CDomainAttributes()
      , isChecked(false), relFiles(), isClientChecked(false), nbConnectedClients_(), indSrv_(), connectedServerRank_()
      , hasBounds(false), hasArea(false), isDistributed_(false), nGlobDomain_(), isUnstructed_(false)
      , global_zoom_ni(0), global_zoom_ibegin(0), global_zoom_nj(0), global_zoom_jbegin(0)
      , isClientAfterTransformationChecked(false), hasLonLat(false)
      , lonvalue_client(), latvalue_client(), bounds_lon_client(), bounds_lat_client()
   { /* Ne rien faire de plus */ }

   CDomain::CDomain(const StdString & id)
      : CObjectTemplate<CDomain>(id), CDomainAttributes()
      , isChecked(false), relFiles(), isClientChecked(false), nbConnectedClients_(), indSrv_(), connectedServerRank_()
      , hasBounds(false), hasArea(false), isDistributed_(false), nGlobDomain_(), isUnstructed_(false)
      , global_zoom_ni(0), global_zoom_ibegin(0), global_zoom_nj(0), global_zoom_jbegin(0)
      , isClientAfterTransformationChecked(false), hasLonLat(false)
      , lonvalue_client(), latvalue_client(), bounds_lon_client(), bounds_lat_client()
   { /* Ne rien faire de plus */ }

   CDomain::~CDomain(void)
   {
   }

   ///---------------------------------------------------------------

   CDomain* CDomain::createDomain()
   {
     CDomain* domain = CDomainGroup::get("domain_definition")->createChild();
     return domain;
   }

   void CDomain::duplicateAttributes(CDomain* domain)
   {
     domain->setAttributes(this);
   }

   const std::set<StdString> & CDomain::getRelFiles(void) const
   {
      return (this->relFiles);
   }

   //----------------------------------------------------------------
   bool CDomain::isEmpty(void) const
   {
      return ((this->zoom_ni_srv == 0) ||
              (this->zoom_nj_srv == 0));
   }

   //----------------------------------------------------------------
   bool CDomain::IsWritten(const StdString & filename) const
   {
      return (this->relFiles.find(filename) != this->relFiles.end());
   }

   //----------------------------------------------------------------
   bool CDomain::isDistributed(void) const
   {
      return isDistributed_;
   }

   //----------------------------------------------------------------
   void CDomain::addRelFile(const StdString & filename)
   {
      this->relFiles.insert(filename);
   }


   StdString CDomain::GetName(void)   { return (StdString("domain")); }
   StdString CDomain::GetDefName(void){ return (CDomain::GetName()); }
   ENodeType CDomain::GetType(void)   { return (eDomain); }

   //----------------------------------------------------------------

   void CDomain::checkDomain(void)
   {
     if (type.isEmpty())
     {
       ERROR("CDomain::checkDomain(void)",
             << "[ Id = " << this->getId() << " ] "
             << "The domain type is not defined,"
             << " check the 'type'  value !")
     }

     if (type==type_attr::unstructured)
     {
        if (ni_glo.isEmpty() || ni_glo <= 0 )
        {
           ERROR("CDomain::checkAttributes(void)",
              << "[ Id = " << this->getId() << " ] "
              << "The global domain is badly defined,"
              << " check the \'ni_glo\'  value !")
        }
        isUnstructed_ = true;
        nj_glo = 1;
        nj = 1;
        jbegin = 0;
        if (ni.isEmpty()) ni = i_index.numElements();
        j_index.resize(ni);
        for(int i=0;i<ni;++i) j_index(i)=0;

        if (!area.isEmpty())
          area.transposeSelf(1, 0);
     }
     else if ((ni_glo.isEmpty() || ni_glo.getValue() <= 0 ) ||
         (nj_glo.isEmpty() || nj_glo.getValue() <= 0 ))
     {
        ERROR("CDomain::checkAttributes(void)",
              << "[ Id = " << this->getId() << " ] "
              << "The global domain is badly defined,"
              << " check the \'ni_glo\' et \'nj_glo\' values !")
     }

     isDistributed_ = !ibegin.isEmpty() || !ni.isEmpty() || !jbegin.isEmpty() || !nj.isEmpty();

     checkLocalIDomain();
     checkLocalJDomain();

     if (i_index.isEmpty())
     {
       i_index.resize(ni*nj);
       for (int j = 0; j < nj; ++j)
         for (int i = 0; i < ni; ++i) i_index(i+j*ni) = i+ibegin;
     }

     if (j_index.isEmpty())
     {
       j_index.resize(ni*nj);
       for (int j = 0; j < nj; ++j)
         for (int i = 0; i < ni; ++i) j_index(i+j*ni) = j+jbegin;
     }
     computeNGlobDomain();

     if (0 == global_zoom_ni) global_zoom_ni = ni_glo;
     if (0 == global_zoom_nj) global_zoom_nj = nj_glo;
   }

   //----------------------------------------------------------------

   void CDomain::checkLocalIDomain(void)
   {
      if (ibegin.isEmpty() && ni.isEmpty())
      {
        ibegin = 0;
        ni = ni_glo;
      }
      else if (!i_index.isEmpty())
      {
        if (ibegin.isEmpty()) ibegin = i_index(0);
      }

      if (ni.getValue() < 0 || ibegin.getValue() < 0 ||
         (ibegin.getValue() + ni.getValue()) > ni_glo.getValue())
      {
        ERROR("CDomain::checkLocalIDomain(void)",
              << "[ Id = " << this->getId() << " ] "
              << "The local domain is wrongly defined,"
              << " check the attributes 'ni_glo', 'ni' and 'ibegin'");
      }
   }

   //----------------------------------------------------------------
   void CDomain::checkLocalJDomain(void)
   {
     if (jbegin.isEmpty() && nj.isEmpty())
     {
       jbegin = 0;
       nj = nj_glo;
     }
     else if (!j_index.isEmpty())
     {
       if (jbegin.isEmpty()) jbegin = j_index(0);
     }

      if (nj.getValue() < 0 || jbegin.getValue() < 0 ||
         (jbegin.getValue() + nj.getValue()) > nj_glo.getValue())
      {
        ERROR("CDomain::checkLocalJDomain(void)",
              << "[ Id = " << this->getId() << " ] "
              << "The local domain is wrongly defined,"
              << " check the attributes 'nj_glo', 'nj' and 'jbegin'");
      }
   }

   //----------------------------------------------------------------
   void CDomain::checkMask(void)
   {
      if (!mask_1d.isEmpty() && !mask_2d.isEmpty())
        ERROR("CDomain::checkMask(void)",
             <<"Only one mask is used but both mask_1d and mask_2d are defined! "<<endl
             <<"Define only one mask: mask_1d or mask_2d ");

      if (!mask_1d.isEmpty() && mask_2d.isEmpty())
      {
        if (mask_1d.numElements() != i_index.numElements())
          ERROR("CDomain::checkMask(void)",
                <<"the mask_1d has not the same size as the local domain"<<endl
                <<"Local size is "<<i_index.numElements()<<endl
                <<"Mask size is "<<mask_1d.numElements());
      }

      if (mask_1d.isEmpty() && !mask_2d.isEmpty())
      {
         if ((mask_2d.extent(0) != ni) ||
             (mask_2d.extent(1) != nj))
            ERROR("CDomain::checkMask(void)",
                  <<"the mask has not the same size as the local domain"<<endl
                  <<"Local size is "<<ni<<"x"<<nj<<endl
                  <<"Mask size is "<<mask_2d.extent(0)<<"x"<<mask_2d.extent(1));
      }

      if (!mask_2d.isEmpty())
      {
        mask_1d.resize(mask_2d.extent(0) * mask_2d.extent(1));
        for (int j = 0; j < nj; ++j)
          for (int i = 0; i < ni; ++i) mask_1d(i+j*ni) = mask_2d(i,j);
        mask_2d.free();
      }
      else if (mask_1d.isEmpty())
      {
        mask_1d.resize(i_index.numElements());
        for (int i = 0; i < i_index.numElements(); ++i) mask_1d(i) = true;
      }
   }


   //----------------------------------------------------------------
   void CDomain::checkDomainData(void)
   {
      if (!data_dim.isEmpty() &&
         !(data_dim.getValue() == 1 || data_dim.getValue() == 2))
      {
         ERROR("CDomain::checkAttributes(void)",
               << "Data dimension incompatible (must be 1 or 2) !") ;
      }
      else if (data_dim.isEmpty())
      {
        data_dim.setValue(1);
      }

      if (data_ibegin.isEmpty())
         data_ibegin.setValue(0) ;
      if (data_jbegin.isEmpty())
         data_jbegin.setValue(0) ;

      if (!data_ni.isEmpty() && (data_ni.getValue() <= 0))
      {
         ERROR("CDomain::checkAttributes(void)",
               << "Data dimension is negative (data_ni).") ;
      }
      else if (data_ni.isEmpty())
      {
         data_ni.setValue((data_dim.getValue() == 1)
                           ? (ni.getValue() * nj.getValue())
                           : ni.getValue());
      }

      if (!data_nj.isEmpty() && (data_nj.getValue() <= 0) )
      {
         ERROR("CDomain::checkAttributes(void)",
               << "Data dimension is negative (data_nj).") ;
      }

      if (data_nj.isEmpty())
         data_nj.setValue((data_dim.getValue() == 1)
                   ? (ni.getValue() * nj.getValue())
                   : nj.getValue());

   }

   //----------------------------------------------------------------
   void CDomain::checkCompression(void)
   {
      if (!data_i_index.isEmpty())
      {
        if (!data_j_index.isEmpty() &&
           (data_j_index.numElements() != data_i_index.numElements()))
        {
           ERROR("CDomain::checkAttributes(void)",
                 <<"Dimension data_j_index incompatible with data_i_index.") ;
        }

       if (2 == data_dim.getValue())
       {
          if (data_j_index.isEmpty())
          {
             ERROR("CDomain::checkAttributes(void)",
                   <<"data_j_index must be defined !") ;
          }
       }
       else // (1 == data_dim.getValue())
       {
          if (data_j_index.isEmpty())
          {
            const int dni = data_ni.getValue();
            data_j_index.resize(dni);
            for (int j = 0; j < dni; ++j) data_j_index(j) = 0;
          }
       }
      }
      else
      {
         if ((data_dim.getValue() == 2) && (!data_j_index.isEmpty()))
            ERROR("CDomain::checkAttributes(void)", << "data_i_index undefined") ;

         if (1 == data_dim.getValue())
         {
            const int dni = data_ni.getValue();
            data_i_index.resize(dni) ;
            data_j_index.resize(dni) ;

            for (int i = 0; i < dni; ++i)
            {
              data_i_index(i) = i;
              data_j_index(i) = 0;
            }
         }
         else   // (data_dim == 2)
         {
            const int dni = data_ni.getValue() * data_nj.getValue();
            data_i_index.resize(dni) ;
            data_j_index.resize(dni) ;

            for(int count = 0, j = 0; j  < data_nj.getValue(); ++j)
            {
               for(int i = 0; i < data_ni.getValue(); ++i, ++count)
               {
                  data_i_index(count) = i;
                  data_j_index(count) = j;
               }
            }
         }
      }
   }

   //----------------------------------------------------------------
   void CDomain::completeLonLatClient(void)
   {
     int i,j,k ;
     CArray<double,1> lonvalue_temp(ni*nj) ;
     CArray<double,2> bounds_lon_temp(nvertex,ni*nj);
     CArray<double,1> latvalue_temp(ni*nj) ;
     CArray<double,2> bounds_lat_temp(nvertex,ni*nj);

     if (!lonvalue_2d.isEmpty())
     {
        for (j = 0; j < nj; ++j)
          for (i = 0; i < ni; ++i)
          {
            lonvalue_temp(i+j*ni) = lonvalue_2d(i,j);
            latvalue_temp(i+j*ni) = latvalue_2d(i,j);
            if (hasBounds)
            {
              k=j*ni+i;
              for(int n=0;n<nvertex;++n)
              {
                bounds_lon_temp(n,k) = bounds_lon_2d(n,i,j);
                bounds_lat_temp(n,k) = bounds_lat_2d(n,i,j);
              }
            }
          }
     }

     if (!lonvalue_1d.isEmpty())
     {
       if (type_attr::rectilinear == type)
       {
         if ((ni == lonvalue_1d.numElements()) && (nj == latvalue_1d.numElements()))
         {
           for(j=0;j<nj;++j)
             for(i=0;i<ni;++i)
             {
               k=j*ni+i;
               lonvalue_temp(k) = lonvalue_1d(i);
               latvalue_temp(k) = latvalue_1d(j);
               if (hasBounds)
               {
                 for(int n=0;n<nvertex;++n)
                 {
                   bounds_lon_temp(n,k) = bounds_lon_1d(n,i);
                   bounds_lat_temp(n,k) = bounds_lat_1d(n,j);
                 }
               }
             }
          }
          else
            ERROR("CDomain::completeLonClient(void)",
                 <<"lonvalue_1d and latvalue_1d has not the same size as the local domain"<<endl
                 <<"Local size is "<<ni<<"x " << nj<< endl
                 <<"lonvalue_1d size is "<<lonvalue_1d.numElements()
                 <<"latvalue_1d size is "<<latvalue_1d.numElements());
       }
       else if (type==type_attr::curvilinear || type==type_attr::unstructured)
       {
         lonvalue_temp=lonvalue_1d;
         latvalue_temp=latvalue_1d;
         if (hasBounds)
         {
           bounds_lon_temp=bounds_lon_1d;
           bounds_lat_temp=bounds_lat_1d;
         }
       }
     }

    int i_ind,j_ind;
    int global_zoom_iend=global_zoom_ibegin+global_zoom_ni-1;
    int global_zoom_jend=global_zoom_jbegin+global_zoom_nj-1;

    int globalIndexCountZoom = 0;
    int nbIndex = i_index.numElements();
    for (i = 0; i < nbIndex; ++i)
    {
      i_ind=i_index(i);
      j_ind=j_index(i);

      if (i_ind >= global_zoom_ibegin && i_ind <= global_zoom_iend && j_ind >= global_zoom_jbegin && j_ind <= global_zoom_jend)
      {
        ++globalIndexCountZoom;
      }
    }

      // Make sure that this attribute is non-empty for every client.
     if (0 != globalIndexCountZoom)
     {
       lonvalue_client.resize(globalIndexCountZoom);
       latvalue_client.resize(globalIndexCountZoom);
       if (hasBounds)
       {
         bounds_lon_client.resize(nvertex,globalIndexCountZoom);
         bounds_lat_client.resize(nvertex,globalIndexCountZoom);
       }
     }

     int nCountZoom = 0;
     for (i = 0; i < nbIndex; ++i)
     {
       i_ind=i_index(i);
       j_ind=j_index(i);

       if (i_ind >= global_zoom_ibegin && i_ind <= global_zoom_iend && j_ind >= global_zoom_jbegin && j_ind <= global_zoom_jend)
       {
         lonvalue_client(nCountZoom) = lonvalue_temp(i);
         latvalue_client(nCountZoom) = latvalue_temp(i);
         if (hasBounds)
         {
           for (int n = 0; n < nvertex; ++n)
           {
             bounds_lon_client(n,nCountZoom) = bounds_lon_temp(n,i);
             bounds_lat_client(n,nCountZoom) = bounds_lat_temp(n,i);
           }
         }
         ++nCountZoom;
       }
     }
   }

   void CDomain::checkBounds(void)
   {
     if (!nvertex.isEmpty() && (0 != nvertex.getValue()))
     {
       if (!bounds_lon_1d.isEmpty() && !bounds_lon_2d.isEmpty())
         ERROR("CDomain::checkBounds(void)",
              <<"Only one longitude boundary value can be used but both bounds_lon_1d and bounds_lon_2d are defined! "<<endl
              <<"Define only one longitude boundary value: lonvalue_1d or bounds_lon_2d ");

       if (!bounds_lat_1d.isEmpty() && !bounds_lat_2d.isEmpty())
         ERROR("CDomain::checkBounds(void)",
              <<"Only one latitude boundary value can be used but both bounds_lat_1d and bounds_lat_2d are defined! "<<endl
              <<"Define only one latitude boundary value: bounds_lat_1d or bounds_lat_2d ");

       if ((!bounds_lon_1d.isEmpty() && bounds_lat_1d.isEmpty()) || (bounds_lon_1d.isEmpty() && !bounds_lat_1d.isEmpty()))
       {
         ERROR("CDomain::checkBounds(void)",
           <<"Only bounds_lon_1d or bounds_lat_1d is defined "<<endl
           <<"Both must be defined ");
       }

       if ((!bounds_lon_2d.isEmpty() && bounds_lat_2d.isEmpty()) || (bounds_lon_2d.isEmpty() && !bounds_lat_2d.isEmpty()))
       {
         ERROR("CDomain::checkBounds(void)",
           <<"Only bounds_lon_2d or bounds_lat_2d is defined "<<endl
           <<"Both must be defined ");
       }

       if (!bounds_lon_1d.isEmpty() && (nvertex.getValue() != bounds_lon_1d.extent(0)))
          ERROR("CDomain::checkBounds(void)",
                <<"Only bounds_lon_1d and nvertex are not compatible"<<endl
                <<"bounds_lon_1d dimension is " << bounds_lon_1d.extent(1)
                <<"but nvertex is " << nvertex.getValue());

       if (!bounds_lon_2d.isEmpty() && (nvertex.getValue() != bounds_lon_2d.extent(0)))
          ERROR("CDomain::checkBounds(void)",
                <<"Only bounds_lon_2d and nvertex are not compatible"<<endl
                <<"bounds_lon_2d dimension is " << bounds_lon_1d.extent(2)
                <<"but nvertex is " << nvertex.getValue());

       if (!bounds_lon_1d.isEmpty() && lonvalue_1d.isEmpty())
           ERROR("CDomain::checkBounds(void)",
                <<"Both bounds_lon_1d and lonvalue_1d must be defined"<<endl);

       if (!bounds_lon_2d.isEmpty() && lonvalue_2d.isEmpty())
           ERROR("CDomain::checkBounds(void)",
                <<"Both bounds_lon_1d and lonvalue_1d must be defined"<<endl);

       if (!bounds_lat_1d.isEmpty() && (nvertex.getValue() != bounds_lat_1d.extent(0)))
          ERROR("CDomain::checkBounds(void)",
                <<"Only bounds_lat_1d and nvertex are not compatible"<<endl
                <<"bounds_lat_1d dimension is " << bounds_lat_1d.extent(1)
                <<"but nvertex is " << nvertex.getValue());

       if (!bounds_lat_2d.isEmpty() && (nvertex.getValue() != bounds_lat_2d.extent(0)))
          ERROR("CDomain::checkBounds(void)",
                <<"Only bounds_lat_2d and nvertex are not compatible"<<endl
                <<"bounds_lat_2d dimension is " << bounds_lat_1d.extent(2)
                <<"but nvertex is " << nvertex.getValue());

       if (!bounds_lat_1d.isEmpty() && latvalue_1d.isEmpty())
           ERROR("CDomain::checkBounds(void)",
                <<"Both bounds_lat_1d and latvalue_1d must be defined"<<endl);

       if (!bounds_lat_2d.isEmpty() && latvalue_2d.isEmpty())
           ERROR("CDomain::checkBounds(void)",
                <<"Both bounds_lat_1d and latvalue_1d must be defined"<<endl);
       hasBounds=true ;
     }
     else
     {
       hasBounds=false;
       nvertex=0 ;
     }
   }

   void CDomain::checkArea(void)
   {
     hasArea = !area.isEmpty();
     if (hasArea)
     {
       if (area.extent(0) != ni || area.extent(1) != nj)
       {
         ERROR("void CDomain::checkArea(void)",
               "The area attribute must be of size ni x nj.");
       }
     }
   }

   void CDomain::checkLonLat()
   {
     hasLonLat = (!latvalue_1d.isEmpty() && !lonvalue_1d.isEmpty()) ||
                 (!latvalue_2d.isEmpty() && !lonvalue_2d.isEmpty());
     if (hasLonLat)
     {
       if (!lonvalue_1d.isEmpty() && !lonvalue_2d.isEmpty())
         ERROR("CDomain::completeLonLatClient(void)",
              <<"Only one longitude value can be used but both lonvalue_1d and lonvalue_2d are defined! "<<endl
              <<"Define only one longitude value: lonvalue_1d or lonvalue_2d ");

       if (!lonvalue_1d.isEmpty() && lonvalue_2d.isEmpty())
       {
         if (lonvalue_1d.numElements() != i_index.numElements())
           ERROR("CDomain::completeLonLatClient(void)",
                 <<"lonvalue_1d has not the same size as the local domain"<<endl
                 <<"Local size is "<<i_index.numElements()<<endl
                 <<"lonvalue_1d size is "<<lonvalue_1d.numElements());
       }

       if (lonvalue_1d.isEmpty() && !lonvalue_2d.isEmpty())
       {
          if ((lonvalue_2d.extent(0) != ni) ||
              (lonvalue_2d.extent(1) != nj))
             ERROR("CDomain::completeLonLatClient(void)",
                   <<"the lonvalue has not the same size as the local domain"<<endl
                   <<"Local size is "<<ni<<"x"<<nj<<endl
                   <<"Lonvalue size is "<<lonvalue_2d.extent(0)<<"x"<<lonvalue_2d.extent(1));
       }

       if (!latvalue_1d.isEmpty() && !latvalue_2d.isEmpty())
         ERROR("CDomain::completeLonLatClient(void)",
              <<"Only one longitude value can be used but both latvalue_1d and latvalue_2d are defined! "<<endl
              <<"Define only one longitude value: latvalue_1d or latvalue_2d ");

       if (!latvalue_1d.isEmpty() && latvalue_2d.isEmpty())
       {
         if (latvalue_1d.numElements() != i_index.numElements())
           ERROR("CDomain::completeLonLatClient(void)",
                 <<"the latvalue_1d has not the same size as the local domain"<<endl
                 <<"Local size is "<<i_index.numElements()<<endl
                 <<"Mask size is "<<latvalue_1d.numElements());
       }

       if (latvalue_1d.isEmpty() && !latvalue_2d.isEmpty())
       {
          if ((latvalue_2d.extent(0) != ni) ||
              (latvalue_2d.extent(1) != nj))
             ERROR("CDomain::completeLonLatClient(void)",
                   <<"the mask has not the same size as the local domain"<<endl
                   <<"Local size is "<<ni<<"x"<<nj<<endl
                   <<"Mask size is "<<latvalue_2d.extent(0)<<"x"<<latvalue_2d.extent(1));
       }
     }
   }

   void CDomain::checkAttributesOnClientAfterTransformation()
   {
     CContext* context=CContext::getCurrent() ;

     if (this->isClientAfterTransformationChecked) return;
     if (context->hasClient)
     {
       this->checkMask();
       if (hasLonLat || hasArea) this->computeConnectedServer();
       if (hasLonLat) this->completeLonLatClient();
     }

     this->isClientAfterTransformationChecked = true;
   }

   //----------------------------------------------------------------
   // Divide function checkAttributes into 2 seperate ones
   // This function only checks all attributes of current domain
   void CDomain::checkAttributesOnClient()
   {
     if (this->isClientChecked) return;
     CContext* context=CContext::getCurrent();

      this->checkDomain();
      this->checkBounds();
      this->checkArea();
      this->checkLonLat();

      if (context->hasClient)
      { // Côté client uniquement
         this->checkMask();
         this->checkDomainData();
         this->checkCompression();
      }
      else
      { // Côté serveur uniquement
      }

      this->isClientChecked = true;
   }

   // Send all checked attributes to server
   void CDomain::sendCheckedAttributes()
   {
     if (!this->isClientChecked) checkAttributesOnClient();
     if (!this->isClientAfterTransformationChecked) checkAttributesOnClientAfterTransformation();
     CContext* context=CContext::getCurrent() ;

     if (this->isChecked) return;
     if (context->hasClient)
     {
       sendServerAttribut();
       if (hasLonLat || hasArea) sendLonLatArea();
     }
     this->isChecked = true;
   }

   void CDomain::checkAttributes(void)
   {
      if (this->isChecked) return;
      CContext* context=CContext::getCurrent() ;

      this->checkDomain();
      this->checkLonLat();
      this->checkBounds();
      this->checkArea();

      if (context->hasClient)
      { // Côté client uniquement
         this->checkMask();
         this->checkDomainData();
         this->checkCompression();

      }
      else
      { // Côté serveur uniquement
      }

      if (context->hasClient)
      {
        this->computeConnectedServer();
        this->completeLonLatClient();
        this->sendServerAttribut();
        this->sendLonLatArea();
      }

      this->isChecked = true;
   }

  void CDomain::sendServerAttribut(void)
  {
    CServerDistributionDescription serverDescription(nGlobDomain_);

    CContext* context = CContext::getCurrent();
    CContextClient* client = context->client;
    int nbServer = client->serverSize;

    if (isUnstructed_) serverDescription.computeServerDistribution(nbServer,0);
    else serverDescription.computeServerDistribution(nbServer,1);

    std::vector<std::vector<int> > serverIndexBegin = serverDescription.getServerIndexBegin();
    std::vector<std::vector<int> > serverDimensionSizes = serverDescription.getServerDimensionSizes();

    CEventClient event(getType(),EVENT_ID_SERVER_ATTRIBUT);
    if (client->isServerLeader())
    {
      std::list<CMessage> msgs;

      const std::list<int>& ranks = client->getRanksServerLeader();
      for (std::list<int>::const_iterator itRank = ranks.begin(), itRankEnd = ranks.end(); itRank != itRankEnd; ++itRank)
      {
        // Use const int to ensure CMessage holds a copy of the value instead of just a reference
        const int ibegin_srv = serverIndexBegin[*itRank][0];
        const int jbegin_srv = serverIndexBegin[*itRank][1];
        const int ni_srv = serverDimensionSizes[*itRank][0];
        const int nj_srv = serverDimensionSizes[*itRank][1];
        const int iend_srv = ibegin_srv + ni_srv - 1;
        const int jend_srv = jbegin_srv + nj_srv - 1;

        msgs.push_back(CMessage());
        CMessage& msg = msgs.back();
        msg << this->getId() ;
        msg << ni_srv << ibegin_srv << iend_srv << nj_srv << jbegin_srv << jend_srv;
        msg << global_zoom_ni << global_zoom_ibegin << global_zoom_nj << global_zoom_jbegin;

        event.push(*itRank,1,msg);
      }
      client->sendEvent(event);
    }
    else client->sendEvent(event);
  }

  void CDomain::computeNGlobDomain()
  {
    nGlobDomain_.resize(2);
    nGlobDomain_[0] = ni_glo.getValue();
    nGlobDomain_[1] = nj_glo.getValue();
  }

  void CDomain::computeConnectedServer(void)
  {
    CContext* context=CContext::getCurrent() ;
    CContextClient* client=context->client ;
    int nbServer=client->serverSize;
    bool doComputeGlobalIndexServer = true;

    int i,j,i_ind,j_ind, nbIndex;
    int global_zoom_iend=global_zoom_ibegin+global_zoom_ni-1 ;
    int global_zoom_jend=global_zoom_jbegin+global_zoom_nj-1 ;

    // Precompute number of index
    int globalIndexCountZoom = 0;
    nbIndex = i_index.numElements();
    for (i = 0; i < nbIndex; ++i)
    {
      i_ind=i_index(i);
      j_ind=j_index(i);

      if (i_ind >= global_zoom_ibegin && i_ind <= global_zoom_iend && j_ind >= global_zoom_jbegin && j_ind <= global_zoom_jend)
      {
        ++globalIndexCountZoom;
      }
    }

    // Fill in index
    CArray<size_t,1> globalIndexDomainZoom(globalIndexCountZoom);
    CArray<size_t,1> localIndexDomainZoom(globalIndexCountZoom);
    CArray<size_t,1> globalIndexDomain(nbIndex);
    size_t globalIndex;
    int globalIndexCount = 0;
    globalIndexCountZoom = 0;

    for (i = 0; i < nbIndex; ++i)
    {
      i_ind=i_index(i);
      j_ind=j_index(i);
      globalIndex = i_ind + j_ind * ni_glo;
      globalIndexDomain(globalIndexCount) = globalIndex;
      ++globalIndexCount;
      if (i_ind >= global_zoom_ibegin && i_ind <= global_zoom_iend && j_ind >= global_zoom_jbegin && j_ind <= global_zoom_jend)
      {
        globalIndexDomainZoom(globalIndexCountZoom) = globalIndex;
        localIndexDomainZoom(globalIndexCountZoom) = i;
        ++globalIndexCountZoom;
      }
    }

     size_t globalSizeIndex = 1, indexBegin, indexEnd;
     int range, clientSize = client->clientSize;
     for (int i = 0; i < nGlobDomain_.size(); ++i) globalSizeIndex *= nGlobDomain_[i];
     indexBegin = 0;
     for (int i = 0; i < clientSize; ++i)
     {
       range = globalSizeIndex / clientSize;
       if (i < (globalSizeIndex%clientSize)) ++range;
       if (i == client->clientRank) break;
       indexBegin += range;
     }
     indexEnd = indexBegin + range - 1;

    CServerDistributionDescription serverDescription(nGlobDomain_);
    if (isUnstructed_) serverDescription.computeServerGlobalIndexInRange(nbServer, std::make_pair<size_t,size_t>(indexBegin, indexEnd), 0);
    else serverDescription.computeServerGlobalIndexInRange(nbServer, std::make_pair<size_t,size_t>(indexBegin, indexEnd), 1);

    CClientServerMapping* clientServerMap = new CClientServerMappingDistributed(serverDescription.getGlobalIndexRange(),
                                                                                client->intraComm);
    clientServerMap->computeServerIndexMapping(globalIndexDomain);
    const std::map<int, std::vector<size_t> >& globalIndexDomainOnServer = clientServerMap->getGlobalIndexOnServer();

    std::map<int, std::vector<size_t> >::const_iterator it = globalIndexDomainOnServer.begin(),
                                                       ite = globalIndexDomainOnServer.end();
    typedef XIOSBinarySearchWithIndex<size_t> BinarySearch;
    std::vector<int>::iterator itVec;

    indSrv_.clear();
    for (; it != ite; ++it)
    {
      int rank = it->first;
      int indexSize = it->second.size();
      std::vector<int> permutIndex(indexSize);
      XIOSAlgorithms::fillInIndex(indexSize, permutIndex);
      XIOSAlgorithms::sortWithIndex<size_t, CVectorStorage>(it->second, permutIndex);
      BinarySearch binSearch(it->second);
      int nb = globalIndexDomainZoom.numElements();
      for (int i = 0; i < nb; ++i)
      {
        if (binSearch.search(permutIndex.begin(), permutIndex.end(), globalIndexDomainZoom(i), itVec))
        {
          indSrv_[rank].push_back(localIndexDomainZoom(i));
        }
      }
    }

    connectedServerRank_.clear();
    for (it = globalIndexDomainOnServer.begin(); it != ite; ++it) {
      connectedServerRank_.push_back(it->first);
    }

    if (!indSrv_.empty())
    {
      connectedServerRank_.clear();
      for (it = indSrv_.begin(); it != indSrv_.end(); ++it)
        connectedServerRank_.push_back(it->first);
    }
    nbConnectedClients_ = clientServerMap->computeConnectedClients(client->serverSize, client->clientSize, client->intraComm, connectedServerRank_);

    delete clientServerMap;
  }

  const std::map<int, vector<size_t> >& CDomain::getIndexServer() const
  {
    return indSrv_;
  }

  /*!
    Send index from client to server(s)
  */
  void CDomain::sendIndex()
  {
    int ns, n, i, j, ind, nv, idx;
    CContext* context = CContext::getCurrent();
    CContextClient* client=context->client;

    CEventClient eventIndex(getType(), EVENT_ID_INDEX);

    list<CMessage> list_msgsIndex;
    list<CArray<int,1> > list_indi, list_indj;

    std::map<int, std::vector<size_t> >::const_iterator it, iteMap;
    iteMap = indSrv_.end();
    for (int k = 0; k < connectedServerRank_.size(); ++k)
    {
      int nbData = 0;
      int rank = connectedServerRank_[k];
      it = indSrv_.find(rank);
      if (iteMap != it)
        nbData = it->second.size();

      list_indi.push_back(CArray<int,1>(nbData));
      list_indj.push_back(CArray<int,1>(nbData));

      CArray<int,1>& indi = list_indi.back();
      CArray<int,1>& indj = list_indj.back();
      const std::vector<size_t>& temp = it->second;
      for (n = 0; n < nbData; ++n)
      {
        idx = static_cast<int>(it->second[n]);
        indi(n) = i_index(idx);
        indj(n) = j_index(idx);
      }

      list_msgsIndex.push_back(CMessage());

      list_msgsIndex.back() << this->getId() << (int)type; // enum ne fonctionne pour les message => ToFix
      list_msgsIndex.back() << isCurvilinear;
      list_msgsIndex.back() << list_indi.back() << list_indj.back();

      eventIndex.push(rank, nbConnectedClients_[rank], list_msgsIndex.back());
    }

    client->sendEvent(eventIndex);
  }

  /*!
    Send area from client to server(s)
  */
  void CDomain::sendArea()
  {
    if (!hasArea) return;

    int ns, n, i, j, ind, nv, idx;
    CContext* context = CContext::getCurrent();
    CContextClient* client=context->client;

    // send area for each connected server
    CEventClient eventArea(getType(), EVENT_ID_AREA);

    list<CMessage> list_msgsArea;
    list<CArray<double,1> > list_area;

    std::map<int, std::vector<size_t> >::const_iterator it, iteMap;
    iteMap = indSrv_.end();
    for (int k = 0; k < connectedServerRank_.size(); ++k)
    {
      int nbData = 0;
      int rank = connectedServerRank_[k];
      it = indSrv_.find(rank);
      if (iteMap != it)
        nbData = it->second.size();
      list_area.push_back(CArray<double,1>(nbData));

      const std::vector<size_t>& temp = it->second;
      for (n = 0; n < nbData; ++n)
      {
        idx = static_cast<int>(it->second[n]);
        i = i_index(idx);
        j = j_index(idx);
        if (hasArea)
          list_area.back()(n) = area(i - ibegin, j - jbegin);
      }

      list_msgsArea.push_back(CMessage());
      list_msgsArea.back() << this->getId() << list_area.back();
      eventArea.push(rank, nbConnectedClients_[rank], list_msgsArea.back());
    }
    client->sendEvent(eventArea);
  }

  /*!
    Send longitude and latitude from client to servers
    Each client send long and lat information to corresponding connected server(s).
    Because longitude and latitude are optional, this function only called if latitude and longitude exist
  */
  void CDomain::sendLonLat()
  {
    if (!hasLonLat) return;

    int ns, n, i, j, ind, nv, idx;
    CContext* context = CContext::getCurrent();
    CContextClient* client=context->client;

    // send lon lat for each connected server
    CEventClient eventLon(getType(), EVENT_ID_LON);
    CEventClient eventLat(getType(), EVENT_ID_LAT);

    list<CMessage> list_msgsLon, list_msgsLat;
    list<CArray<double,1> > list_lon, list_lat;
    list<CArray<double,2> > list_boundslon, list_boundslat;

    std::map<int, std::vector<size_t> >::const_iterator it, iteMap;
    iteMap = indSrv_.end();
    for (int k = 0; k < connectedServerRank_.size(); ++k)
    {
      int nbData = 0;
      int rank = connectedServerRank_[k];
      it = indSrv_.find(rank);
      if (iteMap != it)
        nbData = it->second.size();

      list_lon.push_back(CArray<double,1>(nbData));
      list_lat.push_back(CArray<double,1>(nbData));

      if (hasBounds)
      {
        list_boundslon.push_back(CArray<double,2>(nvertex, nbData));
        list_boundslat.push_back(CArray<double,2>(nvertex, nbData));
      }

      CArray<double,1>& lon = list_lon.back();
      CArray<double,1>& lat = list_lat.back();
      const std::vector<size_t>& temp = it->second;
      for (n = 0; n < nbData; ++n)
      {
        idx = static_cast<int>(it->second[n]);
        lon(n) = lonvalue_client(idx);
        lat(n) = latvalue_client(idx);

        if (hasBounds)
        {
          CArray<double,2>& boundslon = list_boundslon.back();
          CArray<double,2>& boundslat = list_boundslat.back();

          for (nv = 0; nv < nvertex; ++nv)
          {
            boundslon(nv, n) = bounds_lon_client(nv, idx);
            boundslat(nv, n) = bounds_lat_client(nv, idx);
          }
        }
      }

      list_msgsLon.push_back(CMessage());
      list_msgsLat.push_back(CMessage());

      list_msgsLon.back() << this->getId() << list_lon.back();
      list_msgsLat.back() << this->getId() << list_lat.back();

      if (hasBounds)
      {
        list_msgsLon.back() << list_boundslon.back();
        list_msgsLat.back() << list_boundslat.back();
      }

      eventLon.push(rank, nbConnectedClients_[rank], list_msgsLon.back());
      eventLat.push(rank, nbConnectedClients_[rank], list_msgsLat.back());
    }

    client->sendEvent(eventLon);
    client->sendEvent(eventLat);
  }

  /*!
    Send some optional information to server(s)
    In the future, this function can be extended with more optional information to send
  */
  void CDomain::sendLonLatArea(void)
  {
    sendIndex();
    sendLonLat();
    sendArea();
  }

  bool CDomain::dispatchEvent(CEventServer& event)
  {
    if (SuperClass::dispatchEvent(event)) return true;
    else
    {
      switch(event.type)
      {
        case EVENT_ID_SERVER_ATTRIBUT:
          recvServerAttribut(event);
          return true;
          break;
        case EVENT_ID_INDEX:
          recvIndex(event);
          return true;
          break;
        case EVENT_ID_LON:
          recvLon(event);
          return true;
          break;
        case EVENT_ID_LAT:
          recvLat(event);
          return true;
          break;
        case EVENT_ID_AREA:
          recvArea(event);
          return true;
          break;
        default:
          ERROR("bool CContext::dispatchEvent(CEventServer& event)",
                << "Unknown Event");
          return false;
       }
    }
  }

  /*!
    Receive attributes event from clients(s)
    \param[in] event event contain info about rank and associated attributes
  */
  void CDomain::recvServerAttribut(CEventServer& event)
  {
    CBufferIn* buffer=event.subEvents.begin()->buffer;
    string domainId ;
    *buffer>>domainId ;
    get(domainId)->recvServerAttribut(*buffer) ;
  }

  /*!
    Receive attributes from client(s): zoom info and begin and n of each server
    \param[in] rank rank of client source
    \param[in] buffer message containing attributes info
  */
  void CDomain::recvServerAttribut(CBufferIn& buffer)
  {
    buffer >> ni_srv >> ibegin_srv >> iend_srv >> nj_srv >> jbegin_srv >> jend_srv
           >> global_zoom_ni >> global_zoom_ibegin >> global_zoom_nj >> global_zoom_jbegin;

    int zoom_iend = global_zoom_ibegin + global_zoom_ni - 1;
    int zoom_jend = global_zoom_jbegin + global_zoom_nj - 1;

    zoom_ibegin_srv = global_zoom_ibegin > ibegin_srv ? global_zoom_ibegin : ibegin_srv ;
    zoom_iend_srv = zoom_iend < iend_srv ? zoom_iend : iend_srv ;
    zoom_ni_srv=zoom_iend_srv-zoom_ibegin_srv+1 ;

    zoom_jbegin_srv = global_zoom_jbegin > jbegin_srv ? global_zoom_jbegin : jbegin_srv ;
    zoom_jend_srv = zoom_jend < jend_srv ? zoom_jend : jend_srv ;
    zoom_nj_srv=zoom_jend_srv-zoom_jbegin_srv+1 ;

    if (zoom_ni_srv<=0 || zoom_nj_srv<=0)
    {
      zoom_ibegin_srv=0 ; zoom_iend_srv=0 ; zoom_ni_srv=0 ;
      zoom_jbegin_srv=0 ; zoom_jend_srv=0 ; zoom_nj_srv=0 ;
    }
    lonvalue_srv.resize(zoom_ni_srv*zoom_nj_srv) ;
    lonvalue_srv = 0. ;
    latvalue_srv.resize(zoom_ni_srv*zoom_nj_srv) ;
    latvalue_srv = 0. ;
    if (hasBounds)
    {
      bounds_lon_srv.resize(nvertex,zoom_ni_srv*zoom_nj_srv) ;
      bounds_lon_srv = 0. ;
      bounds_lat_srv.resize(nvertex,zoom_ni_srv*zoom_nj_srv) ;
      bounds_lat_srv = 0. ;
    }

    if (hasArea)
      area_srv.resize(zoom_ni_srv * zoom_nj_srv);
  }

  /*!
    Receive index event from clients(s)
    \param[in] event event contain info about rank and associated index
  */
  void CDomain::recvIndex(CEventServer& event)
  {
    list<CEventServer::SSubEvent>::iterator it;
    for (it = event.subEvents.begin(); it != event.subEvents.end(); ++it)
    {
      CBufferIn* buffer = it->buffer;
      string domainId;
      *buffer >> domainId;
      get(domainId)->recvIndex(it->rank, *buffer);
    }
  }

  /*!
    Receive index information from client(s)
    \param[in] rank rank of client source
    \param[in] buffer message containing index info
  */
  void CDomain::recvIndex(int rank, CBufferIn& buffer)
  {
    int type_int;
    buffer >> type_int >> isCurvilinear >> indiSrv[rank] >> indjSrv[rank];
    type.setValue((type_attr::t_enum)type_int); // probleme des type enum avec les buffers : ToFix
  }

  /*!
    Receive longitude event from clients(s)
    \param[in] event event contain info about rank and associated longitude
  */
  void CDomain::recvLon(CEventServer& event)
  {
    list<CEventServer::SSubEvent>::iterator it;
    for (it = event.subEvents.begin(); it != event.subEvents.end(); ++it)
    {
      CBufferIn* buffer = it->buffer;
      string domainId;
      *buffer >> domainId;
      get(domainId)->recvLon(it->rank, *buffer);
    }
  }

  /*!
    Receive longitude information from client(s)
    \param[in] rank rank of client source
    \param[in] buffer message containing longitude info
  */
  void CDomain::recvLon(int rank, CBufferIn& buffer)
  {
    CArray<int,1> &indi = indiSrv[rank], &indj = indjSrv[rank];
    CArray<double,1> lon;
    CArray<double,2> boundslon;

    buffer >> lon;
    if (hasBounds) buffer >> boundslon;

    int i, j, ind_srv;
    for (int ind = 0; ind < indi.numElements(); ind++)
    {
      i = indi(ind); j = indj(ind);
      ind_srv = (i - zoom_ibegin_srv) + (j - zoom_jbegin_srv) * zoom_ni_srv;
      lonvalue_srv(ind_srv) = lon(ind);
      if (hasBounds)
      {
        for (int nv = 0; nv < nvertex; ++nv)
          bounds_lon_srv(nv, ind_srv) = boundslon(nv, ind);
      }
    }
  }

  /*!
    Receive latitude event from clients(s)
    \param[in] event event contain info about rank and associated latitude
  */
  void CDomain::recvLat(CEventServer& event)
  {
    list<CEventServer::SSubEvent>::iterator it;
    for (it = event.subEvents.begin(); it != event.subEvents.end(); ++it)
    {
      CBufferIn* buffer = it->buffer;
      string domainId;
      *buffer >> domainId;
      get(domainId)->recvLat(it->rank, *buffer);
    }
  }

  /*!
    Receive latitude information from client(s)
    \param[in] rank rank of client source
    \param[in] buffer message containing latitude info
  */
  void CDomain::recvLat(int rank, CBufferIn& buffer)
  {
    CArray<int,1> &indi = indiSrv[rank], &indj = indjSrv[rank];
    CArray<double,1> lat;
    CArray<double,2> boundslat;

    buffer >> lat;
    if (hasBounds) buffer >> boundslat;

    int i, j, ind_srv;
    for (int ind = 0; ind < indi.numElements(); ind++)
    {
      i = indi(ind); j = indj(ind);
      ind_srv = (i - zoom_ibegin_srv) + (j - zoom_jbegin_srv) * zoom_ni_srv;
      latvalue_srv(ind_srv) = lat(ind);
      if (hasBounds)
      {
        for (int nv = 0; nv < nvertex; nv++)
          bounds_lat_srv(nv, ind_srv) = boundslat(nv, ind);
      }
    }
  }

  /*!
    Receive area event from clients(s)
    \param[in] event event contain info about rank and associated area
  */
  void CDomain::recvArea(CEventServer& event)
  {
    list<CEventServer::SSubEvent>::iterator it;
    for (it = event.subEvents.begin(); it != event.subEvents.end(); ++it)
    {
      CBufferIn* buffer = it->buffer;
      string domainId;
      *buffer >> domainId;
      get(domainId)->recvArea(it->rank, *buffer);
    }
  }

  /*!
    Receive area information from client(s)
    \param[in] rank rank of client source
    \param[in] buffer message containing area info
  */
  void CDomain::recvArea(int rank, CBufferIn& buffer)
  {
    CArray<int,1> &indi = indiSrv[rank], &indj = indjSrv[rank];
    CArray<double,1> clientArea;

    buffer >> clientArea;

    int i, j, ind_srv;
    for (int ind = 0; ind < indi.numElements(); ind++)
    {
      i = indi(ind); j = indj(ind);
      ind_srv = (i - zoom_ibegin_srv) + (j - zoom_jbegin_srv) * zoom_ni_srv;
      area_srv(ind_srv) = clientArea(ind);
    }
  }

  /*!
    Check whether a domain has transformation
    \return true if domain has transformation
  */
  bool CDomain::hasTransformation()
  {
    return (!transformationMap_.empty());
  }

  /*!
    Set transformation for current domain. It's the method to move transformation in hierarchy
    \param [in] domTrans transformation on domain
  */
  void CDomain::setTransformations(const TransMapTypes& domTrans)
  {
    transformationMap_ = domTrans;
  }

  /*!
    Get all transformation current domain has
    \return all transformation
  */
  CDomain::TransMapTypes CDomain::getAllTransformations(void)
  {
    return transformationMap_;
  }

  /*!
    Check the validity of all transformations applied on domain
  This functions is called AFTER all inherited attributes are solved
  */
  void CDomain::checkTransformations()
  {
    TransMapTypes::const_iterator itb = transformationMap_.begin(), it,
                                  ite = transformationMap_.end();
    for (it = itb; it != ite; ++it)
    {
      (it->second)->checkValid(this);
    }
  }

  /*!
    A current domain will go up the hierarchy to find out the domain from which it has transformation
  */
  void CDomain::solveInheritanceTransformation()
  {
    if (this->hasTransformation()) return;

    std::vector<CDomain*> refDomain;
    CDomain* refer_sptr;
    CDomain* refer_ptr = this;
    while (refer_ptr->hasDirectDomainReference())
    {
      refDomain.push_back(refer_ptr);
      refer_sptr = refer_ptr->getDirectDomainReference();
      refer_ptr  = refer_sptr;
      if (refer_ptr->hasTransformation()) break;
    }

    if (refer_ptr->hasTransformation())
      for (int idx = 0; idx < refDomain.size(); ++idx)
        refDomain[idx]->setTransformations(refer_ptr->getAllTransformations());
  }

  /*!
    Parse children nodes of a domain in xml file.
    Whenver there is a new transformation, its type and name should be added into this function
    \param node child node to process
  */
  void CDomain::parse(xml::CXMLNode & node)
  {
    SuperClass::parse(node);

    if (node.goToChildElement())
    {
      StdString zoomDomainDefRoot("zoom_domain_definition");
      StdString zoom("zoom_domain");
      StdString interpFromFileDomainDefRoot("interpolate_from_file_domain_definition");
      StdString interpFromFile("interpolate_from_file_domain");
      do
      {
        if (node.getElementName() == zoom) {
          CZoomDomain* tmp = (CZoomDomainGroup::get(zoomDomainDefRoot))->createChild();
          tmp->parse(node);
          transformationMap_.push_back(std::make_pair(TRANS_ZOOM_DOMAIN,tmp));
        }
        else if (node.getElementName() == interpFromFile)
        {
          CInterpolateFromFileDomain* tmp = (CInterpolateFromFileDomainGroup::get(interpFromFileDomainDefRoot))->createChild();
          tmp->parse(node);
          transformationMap_.push_back(std::make_pair(TRANS_INTERPOLATE_DOMAIN_FROM_FILE,tmp));
        }
      } while (node.goToNextElement()) ;
      node.goToParentElement();
    }
  }
   //----------------------------------------------------------------

   DEFINE_REF_FUNC(Domain,domain)

   ///---------------------------------------------------------------

} // namespace xios
