#ifndef __XIOS_CObjectFactory_impl__
#define __XIOS_CObjectFactory_impl__

#include "object_factory.hpp"

namespace xios
{
   /// ////////////////////// Définitions ////////////////////// ///
   template <typename U>
       int CObjectFactory::GetObjectNum(void)
   {
      if (CurrContext.size() == 0)
         ERROR("CObjectFactory::GetObjectNum(void)",
               << "please define current context id !");
      return (U::AllVectObj[CObjectFactory::CurrContext].size());
   }

   template <typename U>
      int CObjectFactory::GetObjectIdNum(void)
   {
      if (CurrContext.size() == 0)
         ERROR("CObjectFactory::GetObjectIdNum(void)",
               << "please define current context id !");
      return (U::AllMapObj[CObjectFactory::CurrContext].size());
   }

   template <typename U>
      bool CObjectFactory::HasObject(const StdString & id)
   {
      if (CurrContext.size() == 0)
         ERROR("CObjectFactory::HasObject(const StdString & id)",
               << "[ id = " << id << " ] please define current context id !");
      return (U::AllMapObj[CObjectFactory::CurrContext].find(id) !=
              U::AllMapObj[CObjectFactory::CurrContext].end());
   }

   template <typename U>
      bool CObjectFactory::HasObject(const StdString & context, const StdString & id)
   {
      if (U::AllMapObj.find(context) == U::AllMapObj.end()) return false ;
      else return (U::AllMapObj[context].find(id) !=  U::AllMapObj[context].end());
   }

   template <typename U>
      std::shared_ptr<U> CObjectFactory::GetObject(const U * const object)
   {
      if (CurrContext.size() == 0)
         ERROR("CObjectFactory::GetObject(const U * const object)",
               << "please define current context id !");
      std::vector<std::shared_ptr<U> > & vect =
                     U::AllVectObj[CObjectFactory::CurrContext];

      typename std::vector<std::shared_ptr<U> >::const_iterator
         it = vect.begin(), end = vect.end();

      for (; it != end; it++)
      {
         std::shared_ptr<U> ptr = *it;
         if (ptr.get() == object)
            return (ptr);
      }

      ERROR("CObjectFactory::GetObject(const U * const object)",
               << "[type = " << U::GetName() << ", adress = " << object << "] "
               << "object was not found.");
      return (std::shared_ptr<U>()); // jamais atteint
   }

   template <typename U>
      std::shared_ptr<U> CObjectFactory::GetObject(const StdString & id)
   {
      if (CurrContext.size() == 0)
         ERROR("CObjectFactory::GetObject(const StdString & id)",
               << "[ id = " << id << " ] please define current context id !");
      if (!CObjectFactory::HasObject<U>(id))
         ERROR("CObjectFactory::GetObject(const StdString & id)",
               << "[ id = " << id << ", U = " << U::GetName() << " ] "
               << "object was not found.");
      return (U::AllMapObj[CObjectFactory::CurrContext][id]);
   }

   template <typename U>
      std::shared_ptr<U> CObjectFactory::GetObject(const StdString & context, const StdString & id)
   {
      if (!CObjectFactory::HasObject<U>(context,id))
         ERROR("CObjectFactory::GetObject(const StdString & id)",
               << "[ id = " << id << ", U = " << U::GetName() <<", context = "<<context<< " ] "
               << "object was not found.");
      return (U::AllMapObj[context][id]);
   }

   template <typename U>
   std::shared_ptr<U> CObjectFactory::CreateObject(const StdString& id)
   {
      if (CurrContext.empty())
         ERROR("CObjectFactory::CreateObject(const StdString& id)",
               << "[ id = " << id << " ] please define current context id !");

      if (CObjectFactory::HasObject<U>(id))
      {
         return CObjectFactory::GetObject<U>(id);
      }
      else
      {
         std::shared_ptr<U> value(new U(id.empty() ? CObjectFactory::GenUId<U>() : id));

         U::AllVectObj[CObjectFactory::CurrContext].insert(U::AllVectObj[CObjectFactory::CurrContext].end(), value);
         U::AllMapObj[CObjectFactory::CurrContext].insert(std::make_pair(value->getId(), value));

         return value;
      }
   }

   template <typename U>
   std::shared_ptr<U> CObjectFactory::CreateAlias(const StdString& id, const StdString& alias)
   {
      if (CurrContext.empty())
         ERROR("CObjectFactory::CreateAlias(const StdString& id, const StdString& alias)",
               << "[ id = " << id << " alias = "<<alias<<" ] please define current context id !");

      if (CObjectFactory::HasObject<U>(alias))
      {
         return CObjectFactory::GetObject<U>(alias);
      }
      else
      {
        if (! CObjectFactory::HasObject<U>(id))
        {
            ERROR("CObjectFactory::CreateAlias(const StdString& id, const StdString& alias)",
               << "[ id = " << id << " alias = "<<alias<<" ] object id doesn't exist"); 
        }
        else  
        {
          std::shared_ptr<U> value = CObjectFactory::GetObject<U>(id);  
          U::AllMapObj[CObjectFactory::CurrContext].insert(std::make_pair(alias, value));
          return value;
         }
      }
   }

   template <typename U>
      const std::vector<std::shared_ptr<U> > &
         CObjectFactory::GetObjectVector(const StdString & context)
   {
      return (U::AllVectObj[context]);
   }

   template <typename U>
   const StdString CObjectFactory::GetUIdBase(void)
   {
      StdString base ; 
//      base = "__"+CObjectFactory::CurrContext + "::" + U::GetName() + "_undef_id_";
      base = CObjectFactory::CurrContext + "__" + U::GetName() + "_undef_id_";
      return base;
   }

   template <typename U>
   StdString CObjectFactory::GenUId(void)
   {
      StdOStringStream oss;
      oss << GetUIdBase<U>() << U::GenId[CObjectFactory::CurrContext]++;
      return oss.str();
   }

   template <typename U>
   bool CObjectFactory::IsGenUId(const StdString& id)
   {
      const StdString base = GetUIdBase<U>();
      return (id.size() > base.size() && id.compare(0, base.size(), base) == 0);
   }

   template <typename U> 
   void CObjectFactory::deleteContext(const StdString & context)
   {
     for (auto& v : U::AllVectObj[context]) v.reset() ;
     U::AllVectObj[context].clear() ;
     U::AllVectObj.erase(context) ; 
     for (auto& m : U::AllMapObj[context])  m.second.reset() ;
     U::AllMapObj[context].clear() ;
     U::AllMapObj.erase(context) ;

     U::GenId.erase(context) ;
   }

   template <typename U> 
   void CObjectFactory::deleteAllContexts(void)
   {
     list<StdString> contextList ;
     for(auto& context : U::AllMapObj) contextList.push_back(context.first) ;
     for(auto& context : contextList) deleteContext<U>(context) ;
   }

   
   template <typename U> 
   void CObjectFactory::dumpObjects(void)
   {
     for (auto& context : U::AllMapObj) 
      for(auto& m : context.second)
      {
        info(100)<<"Dump All Object"<<endl ;
        info(100)<<"Object from context "<<context.first<<" with id "<<m.first<<endl ; 
      }
   }
} // namespace xios

#endif // __XIOS_CObjectFactory_impl__
