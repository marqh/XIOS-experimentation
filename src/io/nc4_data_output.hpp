#ifndef __XIOS_NC4_DATA_OUTPUT__
#define __XIOS_NC4_DATA_OUTPUT__

/// XIOS headers ///
#include "xios_spl.hpp"
#include "onetcdf4.hpp"
#include "data_output.hpp"

namespace xios
{
      /// ////////////////////// Déclarations ////////////////////// ///

      class CNc4DataOutput
         : protected CONetCDF4
         , public virtual CDataOutput
      {
         public :

            /// Définition de type ///
            typedef CONetCDF4   SuperClassWriter;
            typedef CDataOutput SuperClass;

            /// Constructeurs ///
            CNc4DataOutput
               (const StdString & filename, bool exist);
            CNc4DataOutput
               (const StdString & filename, bool exist, bool useClassicFormat,
                MPI_Comm comm_file, bool multifile, bool isCollective=true);

            CNc4DataOutput(const CNc4DataOutput & dataoutput);       // Not implemented.
            CNc4DataOutput(const CNc4DataOutput * const dataoutput); // Not implemented.

            /// Accesseur ///
            const StdString & getFileName(void) const;

            /// Destructeur ///
            virtual ~CNc4DataOutput(void);
            bool singleDomain ;
            bool isCollective ;
         protected :

            /// Ecriture ///
            virtual void writeDomain_   (CDomain* domain);
            virtual void writeAxis_     (CAxis* axis);
            virtual void writeGridCompressed_(CGrid* grid);
            virtual void writeTimeDimension_(void);
            virtual void writeField_    (CField* field);
            virtual void writeAttribute_(CVariable* var);
            virtual void writeAttribute_(CVariable* var, const string& fieldId);
            virtual void writeFieldData_(CField* field);
            virtual void writeFile_     (CFile* file);
            virtual void closeFile_     (void);
            virtual void syncFile_      (void);
            virtual void writeTimeAxis_ (CField* field,
                                         const boost::shared_ptr<CCalendar> cal);

         protected :
            void writeUnstructuredDomain (CDomain* domain);
            void writeLocalAttributes(int ibegin, int ni, int jbegin, int nj, StdString domid);
            void writeLocalAttributes_IOIPSL(const StdString& dimXid, const StdString& dimYid,
                                             int ibegin, int ni, int jbegin, int nj, int ni_glo, int nj_glo, int rank, int size);
            void writeTimeAxisAttributes(const StdString & axis_name,
                                         const StdString & calendar,
                                         const StdString & units,
                                         const StdString & time_origin,
                                         const StdString & time_bounds   = StdString("bounds"),
                                         const StdString & standard_name = StdString("time"),
                                         const StdString & long_name     = StdString("Time axis"));

            void writeFileAttributes(const StdString & name,
                                     const StdString & description,
                                     const StdString & conventions,
                                     const StdString & production,
                                     const StdString & timeStamp);

            void writeMaskAttributes(const StdString & mask_name,
                                     int data_dim,
                                     int data_ni     = 0,
                                     int data_nj     = 0,
                                     int data_ibegin = 0,
                                     int data_jbegin = 0);

            void writeAxisAttributes(const StdString & axis_name,
                                     const StdString & axis,
                                     const StdString & standard_name,
                                     const StdString & long_name,
                                     const StdString & units,
                                     const StdString & nav_model);

         private :

            /// Traitement ///
            StdString getTimeStamp(void) const;

            /// Propriétés privées ///
            MPI_Comm comm_file;
            const StdString filename;
            std::set<std::string> writtenDomains ; 
            std::set<std::string> writtenAxis ;
            bool isWrittenDomain(const std::string& domainName) { return this->writtenDomains.find(domainName) != this->writtenDomains.end(); }
            bool isWrittenAxis(const std::string& axisName) { return this->writtenAxis.find(axisName) != this->writtenAxis.end(); }

      }; // class CNc4DataOutput

} // namespace xios

#endif //__XIOS_NC4_DATA_OUTPUT__
