#include "Save.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <unordered_set>
#ifdef _WIN32
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#endif

namespace MyGame
{
    namespace
    {
        constexpr std::size_t MaxPayload=32768;
        constexpr int MaxTargets=512;

        void WriteInventory(std::ostream& out,const Inventory& inventory)
        {
            out<<inventory.UsedSlots()<<' ';
            for(const auto& stack:inventory.Slots())
                if(stack.quantity>0) out<<static_cast<unsigned int>(stack.item)<<' '<<stack.quantity<<' ';
        }

        bool ReadInventory(std::istream& in,Inventory& inventory)
        {
            int count=0;
            if(!(in>>count) || count<0 || count>inventory.SlotCount()) return false;
            for(int i=0;i<count;++i)
            {
                unsigned int id=0;
                int quantity=0;
                if(!(in>>id>>quantity)) return false;
                const auto item=static_cast<ItemId>(id);
                const auto* definition=FindItem(item);
                if(!definition || quantity<1 || quantity>definition->maxStack ||
                   !inventory.TryAdd(item,quantity)) return false;
            }
            return true;
        }
        bool Finite(glm::vec2 point)
        {
            return std::isfinite(point.x) && std::isfinite(point.y);
        }

#ifdef _WIN32
        struct Connection
        {
            SQLHENV environment=SQL_NULL_HENV;
            SQLHDBC connection=SQL_NULL_HDBC;
            SQLHSTMT statement=SQL_NULL_HSTMT;
            bool connected=false;

            ~Connection()
            {
                if(statement) SQLFreeHandle(SQL_HANDLE_STMT,statement);
                if(connected) SQLDisconnect(connection);
                if(connection) SQLFreeHandle(SQL_HANDLE_DBC,connection);
                if(environment) SQLFreeHandle(SQL_HANDLE_ENV,environment);
            }

            bool Open()
            {
                if(!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&environment)) ||
                   !SQL_SUCCEEDED(SQLSetEnvAttr(environment,SQL_ATTR_ODBC_VERSION,
                                                reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3),0)) ||
                   !SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_DBC,environment,&connection))) return false;
                if(!SQL_SUCCEEDED(SQLSetConnectAttr(connection,SQL_ATTR_LOGIN_TIMEOUT,
                                                  reinterpret_cast<SQLPOINTER>(3),0))) return false;
                SQLCHAR settings[]=
                    "Driver={ODBC Driver 18 for SQL Server};Server=lpc:.\\SQLEXPRESS;"
                    "Database=MyFarmEngineLocal;Trusted_Connection=Yes;Encrypt=Yes;TrustServerCertificate=Yes;LongAsMax=Yes;";
                connected=SQL_SUCCEEDED(SQLDriverConnectA(connection,nullptr,settings,SQL_NTS,
                                                         nullptr,0,nullptr,SQL_DRIVER_NOPROMPT));
                return connected && SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT,connection,&statement)) &&
                       SQL_SUCCEEDED(SQLSetStmtAttr(statement,SQL_ATTR_QUERY_TIMEOUT,reinterpret_cast<SQLPOINTER>(3),0));
            }
            bool Execute(std::string query)
            {
                return SQL_SUCCEEDED(SQLExecDirectA(statement,reinterpret_cast<SQLCHAR*>(query.data()),SQL_NTS));
            }
        };
#endif
    }

    std::string EncodeSave(const FarmSave& state)
    {
        std::ostringstream out;
        out.imbue(std::locale::classic());
        out<<std::setprecision(std::numeric_limits<double>::max_digits10);
        out<<"MYFARM "<<(state.portals.empty()?1:2)<<' '<<state.position.x<<' '<<state.position.y<<' '<<state.money<<' '
           <<state.selectedSlot<<' '<<static_cast<unsigned int>(state.seed)<<' ';
        WriteInventory(out,state.bag);
        out<<state.home.enabled<<' '<<state.home.ownerId<<' '<<state.home.origin.x<<' '
           <<state.home.origin.y<<' '<<state.home.size.x<<' '<<state.home.size.y<<' ';
        WriteInventory(out,state.home.items);
        out<<state.targets.size()<<' ';
        for(const auto& target:state.targets)
            out<<std::quoted(target.id)<<' '<<static_cast<int>(target.kind)<<' '<<target.active<<' '
               <<target.health<<' '<<target.tilled<<' '<<target.planted<<' '<<target.ready<<' '
               <<static_cast<unsigned int>(target.crop)<<' '<<target.age<<' '<<target.moisture<<' '
               <<target.revision<<' '<<target.stock<<' ';
        if(!state.portals.empty())
        {
            out<<state.portals.size()<<' ';
            for(const auto& portal:state.portals) out<<std::quoted(portal.id)<<' '<<portal.open<<' ';
        }
        return out.str();
    }

    std::optional<FarmSave> DecodeSave(std::string_view payload)
    {
        if(payload.empty() || payload.size()>MaxPayload) return std::nullopt;
        std::istringstream in{std::string(payload)};
        in.imbue(std::locale::classic());
        FarmSave save;
        std::string magic;
        int version=0;
        unsigned int seed=0;
        if(!(in>>magic>>version>>save.position.x>>save.position.y>>save.money>>save.selectedSlot>>seed) ||
           magic!="MYFARM" || (version!=1 && version!=2) || !Finite(save.position) || save.money<0 ||
           save.selectedSlot<0 || save.selectedSlot>=4) return std::nullopt;
        save.seed=static_cast<ItemId>(seed);
        if(!CropFromSeed(save.seed) || !ReadInventory(in,save.bag)) return std::nullopt;
        auto& home=save.home;
        if(!(in>>home.enabled>>home.ownerId>>home.origin.x>>home.origin.y>>home.size.x>>home.size.y) ||
           !Finite(home.origin) || !Finite(home.size) || home.ownerId!=1 ||
           home.size.x<=0 || home.size.y<=0 || !ReadInventory(in,home.items)) return std::nullopt;
        int count=0;
        if(!(in>>count) || count<0 || count>MaxTargets) return std::nullopt;
        std::unordered_set<std::string> ids;
        for(int i=0;i<count;++i)
        {
            InteractionTarget target;
            int kind=0;
            unsigned int crop=0;
            if(!(in>>std::quoted(target.id)>>kind>>target.active>>target.health>>target.tilled>>target.planted>>
                 target.ready>>crop>>target.age>>target.moisture>>target.revision>>target.stock) ||
               target.id.empty() || target.id.size()>128 || !ids.insert(target.id).second ||
               kind<0 || kind>static_cast<int>(TargetKind::FruitTree) || target.health<0 ||
               !std::isfinite(target.age) || !std::isfinite(target.moisture) ||
               target.age<0 || target.moisture<0 || target.stock<0) return std::nullopt;
            target.kind=static_cast<TargetKind>(kind);
            target.crop=static_cast<CropId>(crop);
            if(target.kind==TargetKind::Crop && target.planted)
            {
                const auto* definition=FindCrop(target.crop);
                if(!definition || !target.tilled || target.age>definition->growthSeconds ||
                   target.moisture>definition->waterSeconds ||
                   target.ready!=(target.age>=definition->growthSeconds)) return std::nullopt;
            }
            else if(target.crop!=CropId::None || target.planted || target.ready ||
                    target.moisture!=0 || (target.kind!=TargetKind::FruitTree && target.age!=0))
                return std::nullopt;
            save.targets.push_back(std::move(target));
        }
        if(version==2)
        {
            int portalCount=0;std::unordered_set<std::string> portalIds;
            if(!(in>>portalCount) || portalCount<1 || portalCount>64) return std::nullopt;
            for(int i=0;i<portalCount;++i)
            {
                SavedPortal portal;
                if(!(in>>std::quoted(portal.id)>>portal.open) || portal.id.empty() || portal.id.size()>128 ||
                   !portalIds.insert(portal.id).second) return std::nullopt;
                save.portals.push_back(std::move(portal));
            }
        }
        in>>std::ws;
        if(!in.eof()) return std::nullopt;
        return save;
    }

    SaveResult SaveManager::Write(const FarmSave& state,bool testSlot)
    {
#ifdef _WIN32
        auto payload=EncodeSave(state);
        if(!DecodeSave(payload)) return {"SAVE INVALID"};
        Connection db;
        if(!db.Open()) return {"SQL OFFLINE"};
        SQLLEN size=static_cast<SQLLEN>(payload.size());
        if(!SQL_SUCCEEDED(SQLBindParameter(db.statement,1,SQL_PARAM_INPUT,SQL_C_CHAR,SQL_LONGVARCHAR,
                                           payload.size(),0,payload.data(),size,&size)) ||
           !db.Execute(testSlot?
              "UPDATE dbo.FarmSave SET previous_payload=payload,payload=? WHERE slot=2":
              "UPDATE dbo.FarmSave SET previous_payload=payload,payload=? WHERE slot=1")) return {"SAVE ERROR"};
        SQLLEN rows=0;
        if(!SQL_SUCCEEDED(SQLRowCount(db.statement,&rows)) || rows!=1) return {"SAVE ERROR"};
        return {"DA LUU",std::nullopt,true};
#else
        (void)state;
        (void)testSlot;
        return {"SQL OFFLINE"};
#endif
    }

    SaveResult SaveManager::Read(bool testSlot)
    {
#ifdef _WIN32
        Connection db;
        if(!db.Open()) return {"SQL OFFLINE"};
        if(!db.Execute(testSlot?"SELECT payload FROM dbo.FarmSave WHERE slot=2":
                                "SELECT payload FROM dbo.FarmSave WHERE slot=1")) return {"LOAD ERROR"};
        const auto fetch=SQLFetch(db.statement);
        if(fetch==SQL_NO_DATA) return {"CHUA LUU"};
        if(!SQL_SUCCEEDED(fetch)) return {"LOAD ERROR"};
        std::array<char,MaxPayload+1> buffer{};
        SQLLEN size=0;
        const auto status=SQLGetData(db.statement,1,SQL_C_CHAR,buffer.data(),buffer.size(),&size);
        if(!SQL_SUCCEEDED(status)) return {"LOAD ERROR"};
        if(size==SQL_NULL_DATA) return {"CHUA LUU"};
        if(status!=SQL_SUCCESS || size<0 || static_cast<std::size_t>(size)>MaxPayload) return {"SAVE INVALID"};
        auto state=DecodeSave({buffer.data(),static_cast<std::size_t>(size)});
        if(!state) return {"SAVE INVALID"};
        return {"DA TAI",std::move(state),true};
#else
        (void)testSlot;
        return {"SQL OFFLINE"};
#endif
    }
}
