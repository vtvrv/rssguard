// For license of this file, see <project-root-folder>/LICENSE.md.

#include "core/messagefilter.h"

#include "core/message.h"

#include <QJSEngine>

MessageFilter::MessageFilter(int id, QObject* parent) : QObject(parent), m_id(id) {}

FilteringAction MessageFilter::filterMessage(QJSEngine* engine) {
  // NOTE: Filter is represented by JavaScript code, each filter must define
  // function with "filterMessage()" prototype. There is a global "msg" object
  // representing "message" available.
  //
  // All standard classes/functions as specified by ECMA-262 are available.
  //
  // MessageObject "msg" global object has some writable properties such as "title" or "author",
  // see core/message.h file for more info.
  //
  // Note that function "filterMessage() must return integer values corresponding
  // to enumeration "FilteringAction" (see file core/message.h).
  // Also, there is a method MessageObject.isDuplicateWithAttribute(int) which is callable
  // with "msg" variable and this method checks if given message already exists in
  // RSS Guard's database. Method is parameterized and the parameter is integer representation
  // of DuplicationAttributeCheck enumeration (see file core/message.h).
  //
  // Example filtering script might look like this:

  /*
      function helper() {
        if (msg.title.includes("A")) {
          msg.isImportant = true;
        }

        return 1;
      }

      function filterMessage() {
        return helper();
      }
   */
  QJSValue filter_func = engine->evaluate(m_script);

  if (filter_func.isError()) {
    qCritical("Error when evaluating script from filter '%d'. Error is: '%s'", id(), qPrintable(filter_func.toString()));
    return FilteringAction::Accept;
  }

  auto filter_output = engine->evaluate(QSL("filterMessage()"));

  if (filter_output.isError()) {
    qCritical("Error when calling filtering function '%d'. Error is: '%s'", id(), qPrintable(filter_output.toString()));
    return FilteringAction::Accept;
  }

  return FilteringAction(filter_output.toInt());
}

int MessageFilter::id() const {
  return m_id;
}

QString MessageFilter::name() const {
  return m_name;
}

void MessageFilter::setName(const QString& name) {
  m_name = name;
}

QString MessageFilter::script() const {
  return m_script;
}

void MessageFilter::setScript(const QString& script) {
  m_script = script;
}
