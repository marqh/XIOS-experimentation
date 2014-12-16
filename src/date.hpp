#ifndef __XMLIO_CDate__
#define __XMLIO_CDate__

/// xios headers ///
#include "xmlioserver_spl.hpp"
#include "exception.hpp"
#include "duration.hpp"

namespace xios
{
      /// ////////////////////// Déclarations ////////////////////// ///
      class CCalendar;

      class CDate
      {
         public :

            /// Constructeurs ///
            //!< Create an empty date associated to no calendar
            CDate(void);
            //!< Create an empty date associated to the specified calendar
            CDate(const CCalendar & cal);
            //!< Create a date associated to the specified calendar
            CDate(const CCalendar & cal, int yr, int mth, int d,
                                         int hr = 0, int min = 0, int sec = 0);
            CDate(const CDate & odate);
            CDate(const CDate * const odate); // Not implemented yet

            /// Destructeur ///
            ~CDate(void);

            /// Opérateurs ///
            CDate & operator=(const CDate & date);
            friend StdOStream & operator<<(StdOStream & out, const CDate & date);
            friend StdIStream & operator>>(StdIStream & in, CDate & date); // Non testée.

            //!< Return the number of seconds since the time origin fixed when creating the calendar
            operator Time(void) const;

            /// Traitements ///
            bool checkDate(void); // Vérifie la validité de la date.

            /// Divers accesseurs ///
            int getYear  (void) const;
            int getMonth (void) const;
            int getDay   (void) const;
            int getHour  (void) const;
            int getMinute(void) const;
            int getSecond(void) const;

            //!< Get the calendar associated to the date
            const CCalendar& getRelCalendar(void) const;
            bool hasRelCalendar(void) const;

            /// Mutateurs ///
            void setYear  (int newyear);
            void setMonth (int newmonth);
            void setDay   (int newday);
            void setHour  (int newhour);
            void setMinute(int newminute);
            void setSecond(int newsecond);

            void setDate(int yr, int mth, int d,
                         int hr = 0, int min = 0, int sec = 0);

            void addMonth (int value);

            //!< Set the calendar associated to the date
            bool setRelCalendar(const CCalendar& relCalendar);

            /// Autres ///
            StdString toString(void) const;
            StdString getStryyyymmdd(void) const;
            string getStr(const string& str) const;


         public : /* static */

            static CDate FromString(const StdString & str, const CCalendar & calendar);

         private :

            /// Propriétés privées ///
            const CCalendar* relCalendar; //!< Calendar associated to the date
            int year, month, day, hour, minute, second; // Année, mois, ...


      }; // class CDate;

} // namespace xios

#endif // __XMLIO_CDate__
