#ifndef CONVENTIONS_H
#define CONVENTIONS_H

#include "string_constant.h"
#include "type_registry.h"
#include "static_counter.h"
#include "debug_and_sql.h"
#include "exceptions.h"
#ifndef SWIG
#include <QtCore>
#endif //SWIG
#include "Engine.h"

#include <cstring>
#include <map>
#include <stdexcept>
#include <string>

#define DECLARE_CONVENTION_TYPE(name_, value_) \
  DECLARE_STRING_CONSTANT(name_, value_) \
  namespace Private \
  { \
    GET_NEXT_STATIC_COUNTER(Counter_ae, Id__##name_) \
    REGISTRY_ADD_TYPE(Registry_ae, Counter_ae::Id__##name_, \
      AE::name_) \
  }

namespace AE {

#ifndef SWIG
const char *printable(const variant &v);
const char *printable(const QVariant &v);
QVariant fromAeVariant(const AE::variant &ae_val);
variant fromQVariant(const QVariant &q_val);
#endif //SWIG
DECLARE_RUNTIME_EXCEPTION(AECommonError)

namespace Private  {
	INIT_STATIC_COUNTER(Counter_ae, 200)
	DECLARE_TYPE_REGISTRY(Registry_ae)
} //namespace Private
//global
DECLARE_STRING_CONSTANT(g_achivements_path, /home/ilia/.local/share/action_engine)
DECLARE_STRING_CONSTANT(g_dbName, ae.db)
DECLARE_STRING_CONSTANT(g_achivementsFileName, achivements.xml)
//Tables
DECLARE_CONVENTION_TYPE(t_users, Users);
DECLARE_CONVENTION_TYPE(t_projects, Projects);
DECLARE_CONVENTION_TYPE(t_sessions, Sessions);
DECLARE_CONVENTION_TYPE(t_actions, Actions);
DECLARE_CONVENTION_TYPE(t_achivements_list, AchivementsList);
DECLARE_CONVENTION_TYPE(t_achivements_done, AchivementsDone);
//Fields
DECLARE_CONVENTION_TYPE(f_id, id);
DECLARE_CONVENTION_TYPE(f_ach_id, AchivementId);
DECLARE_CONVENTION_TYPE(f_start, Start);
DECLARE_CONVENTION_TYPE(f_finish, Finish);
DECLARE_CONVENTION_TYPE(f_time, Time);
DECLARE_CONVENTION_TYPE(f_actTime, ActionTime);
DECLARE_CONVENTION_TYPE(f_session, Session);
DECLARE_CONVENTION_TYPE(f_name, Name);
DECLARE_CONVENTION_TYPE(f_user, User);
DECLARE_CONVENTION_TYPE(f_project, Project);
DECLARE_CONVENTION_TYPE(f_passwd, Password);
DECLARE_CONVENTION_TYPE(f_session_id, SessionId);
DECLARE_CONVENTION_TYPE(f_description, Description);
DECLARE_CONVENTION_TYPE(f_condition, Condition);
DECLARE_CONVENTION_TYPE(f_visible, Visible);
DECLARE_CONVENTION_TYPE(f_type, Type);
//Tags
DECLARE_STRING_CONSTANT(tag_element, achivement);
DECLARE_STRING_CONSTANT(tag_lastId, lastId);
DECLARE_STRING_CONSTANT(tag_name, name);
DECLARE_STRING_CONSTANT(tag_value, value);
DECLARE_STRING_CONSTANT(tag_root, root);
DECLARE_STRING_CONSTANT(tag_type_str, type_str);
DECLARE_STRING_CONSTANT(tag_description, Description)
DECLARE_STRING_CONSTANT(tag_content, content);
DECLARE_STRING_CONSTANT(tag_project, project);
DECLARE_STRING_CONSTANT(tag_user, user);
DECLARE_STRING_CONSTANT(tag_removed, removed);
DECLARE_STRING_CONSTANT(val_type_sql, sql);
//helpers

//Coming from external
DECLARE_CONVENTION_TYPE(f_statement, Statement);
DECLARE_CONVENTION_TYPE(f_result, Result);
DECLARE_CONVENTION_TYPE(f_success, Success);
//Network
DECLARE_STRING_CONSTANT(n_tables_path, /TablesPath);
DECLARE_STRING_CONSTANT(n_fields_path, /FieldsPath);
DECLARE_STRING_CONSTANT(n_achievement_list_path_get, /AchievementListGet)
DECLARE_STRING_CONSTANT(n_achievement_list_path_send, /AchievementListSend)
DECLARE_STRING_CONSTANT(n_post_content, Content)

typedef std::map<char const*, char const*> conv_map;
	namespace Private {
		GET_NEXT_STATIC_COUNTER(Counter_ae, LastTypeCounter);
		template <unsigned N>
		struct FillMap {
			static void Fill(conv_map &m) {
				FillMap<N - 1>::Fill(m);
				m[Registry_ae<N>::Type::Name] = Registry_ae<N>::Type::Value;
			}
		};
		template <>
		struct FillMap<0> {
			static void Fill(conv_map &) {
			}
		};
		inline void fillConventions(conv_map &m) {
			FillMap<Counter_ae::LastTypeCounter - 1>::Fill(m);
		}
	} // namespace Private

inline void fillConventions(conv_map &m) {
	Private::fillConventions(m);
}

} // namespace AE

#endif //CONVENTIONS_H
