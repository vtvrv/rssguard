#include <QDateTime>
#include <QJSEngine>
#include <QProcess>

// For license of this file, see <project-root-folder>/LICENSE.md.

#include "gui/dialogs/formmessagefiltersmanager.h"

#include "core/messagefilter.h"
#include "exceptions/filteringexception.h"
#include "gui/guiutilities.h"
#include "gui/messagebox.h"
#include "miscellaneous/application.h"
#include "miscellaneous/feedreader.h"
#include "miscellaneous/iconfactory.h"
#include "network-web/webfactory.h"
#include "services/abstract/accountcheckmodel.h"

FormMessageFiltersManager::FormMessageFiltersManager(FeedReader* reader, const QList<ServiceRoot*>& accounts, QWidget* parent)
  : QDialog(parent), m_feedsModel(new AccountCheckModel(this)), m_rootItem(new RootItem()),
  m_accounts(accounts), m_reader(reader), m_loadingFilter(false) {
  m_ui.setupUi(this);

  GuiUtilities::applyDialogProperties(*this, qApp->icons()->fromTheme(QSL("view-list-details")));

  m_ui.m_treeFeeds->setModel(m_feedsModel);
  m_ui.m_btnAddNew->setIcon(qApp->icons()->fromTheme(QSL("list-add")));
  m_ui.m_btnRemoveSelected->setIcon(qApp->icons()->fromTheme(QSL("list-remove")));
  m_ui.m_btnBeautify->setIcon(qApp->icons()->fromTheme(QSL("format-justify-fill")));
  m_ui.m_btnTest->setIcon(qApp->icons()->fromTheme(QSL("media-playback-start")));
  m_ui.m_btnDetailedHelp->setIcon(qApp->icons()->fromTheme(QSL("help-contents")));
  m_ui.m_txtScript->setFont(QFontDatabase::systemFont(QFontDatabase::SystemFont::FixedFont));

  connect(m_ui.m_btnDetailedHelp, &QPushButton::clicked, this, []() {
    qApp->web()->openUrlInExternalBrowser(MSG_FILTERING_HELP);
  });
  connect(m_ui.m_listFilters, &QListWidget::currentRowChanged,
          this, &FormMessageFiltersManager::loadFilter);
  connect(m_ui.m_btnAddNew, &QPushButton::clicked,
          this, &FormMessageFiltersManager::addNewFilter);
  connect(m_ui.m_txtTitle, &QLineEdit::textChanged, this, &FormMessageFiltersManager::saveSelectedFilter);
  connect(m_ui.m_txtScript, &QPlainTextEdit::textChanged, this, &FormMessageFiltersManager::saveSelectedFilter);
  connect(m_ui.m_btnTest, &QPushButton::clicked, this, &FormMessageFiltersManager::testFilter);
  connect(m_ui.m_btnBeautify, &QPushButton::clicked, this, &FormMessageFiltersManager::beautifyScript);
  connect(m_ui.m_cmbAccounts, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this]() {
    // Load feeds/categories of the account and check marks.
    loadSelectedAccount();
    loadFilterFeedAssignments();
  });

  initializeTestingMessage();
  loadFilter();
  loadAccounts();
}

FormMessageFiltersManager::~FormMessageFiltersManager() {
  delete m_rootItem;
}

MessageFilter* FormMessageFiltersManager::selectedFilter() const {
  if (m_ui.m_listFilters->currentItem() == nullptr) {
    return nullptr;
  }
  else {
    return m_ui.m_listFilters->currentItem()->data(Qt::ItemDataRole::UserRole).value<MessageFilter*>();
  }
}

ServiceRoot* FormMessageFiltersManager::selectedAccount() const {
  auto dat = m_ui.m_cmbAccounts->currentData(Qt::ItemDataRole::UserRole);

  return dat.isNull() ? nullptr : dat.value<ServiceRoot*>();
}

void FormMessageFiltersManager::addNewFilter() {
  auto* fltr = m_reader->addMessageFilter(
    tr("New message filter"),
    QSL("function filterMessage() { return 1; }"));
  auto* it = new QListWidgetItem(fltr->name(), m_ui.m_listFilters);

  it->setData(Qt::ItemDataRole::UserRole, QVariant::fromValue<MessageFilter*>(fltr));

  m_ui.m_listFilters->setCurrentRow(m_ui.m_listFilters->count() - 1);
}

void FormMessageFiltersManager::saveSelectedFilter() {
  if (m_loadingFilter) {
    return;
  }

  auto* fltr = selectedFilter();

  if (fltr == nullptr || m_ui.m_txtTitle->text().isEmpty() || m_ui.m_txtScript->toPlainText().isEmpty()) {
    return;
  }

  fltr->setName(m_ui.m_txtTitle->text());
  fltr->setScript(m_ui.m_txtScript->toPlainText());
  m_ui.m_listFilters->currentItem()->setText(fltr->name());

  m_reader->updateMessageFilter(fltr);
}

void FormMessageFiltersManager::loadFilter() {
  auto* filter = selectedFilter();
  auto* acc = selectedAccount();

  showFilter(filter);
}

void FormMessageFiltersManager::testFilter() {
  // Perform per-message filtering.
  QJSEngine filter_engine;
  QSqlDatabase database = qApp->database()->connection(metaObject()->className());

  // Create JavaScript communication wrapper for the message.
  MessageObject msg_obj(&database, QString::number(NO_PARENT_CATEGORY), NO_PARENT_CATEGORY);

  // Register the wrapper.
  auto js_object = filter_engine.newQObject(&msg_obj);

  filter_engine.installExtensions(QJSEngine::Extension::ConsoleExtension);
  filter_engine.globalObject().setProperty("msg", js_object);

  Message msg = testingMessage();

  msg_obj.setMessage(&msg);

  auto* fltr = selectedFilter();

  try {
    FilteringAction decision = fltr->filterMessage(&filter_engine);

    m_ui.m_txtErrors->setTextColor(decision == FilteringAction::Accept ? Qt::GlobalColor::darkGreen : Qt::GlobalColor::red);

    QString answer = tr("Message will be %1.\n\n").arg(decision == FilteringAction::Accept
                                                       ? tr("ACCEPTED")
                                                       : tr("REJECTED"));

    answer += tr("Output (modified) message is:\n"
                 "  Title = '%1'\n"
                 "  URL = '%2'\n"
                 "  Author = '%3'\n"
                 "  Is read/important = '%4/%5'\n"
                 "  Created on = '%6'\n"
                 "  Contents = '%7'").arg(msg.m_title, msg.m_url, msg.m_author,
                                          msg.m_isRead ? tr("yes") : tr("no"),
                                          msg.m_isImportant ? tr("yes") : tr("no"),
                                          QString::number(msg.m_created.toMSecsSinceEpoch()),
                                          msg.m_contents);

    m_ui.m_txtErrors->setPlainText(answer);
  }
  catch (const FilteringException& ex) {
    m_ui.m_txtErrors->setTextColor(Qt::GlobalColor::red);
    m_ui.m_txtErrors->setPlainText(tr("JavaScript-based filter contains errors: '%1'.").arg(ex.message()));
  }

  // See output.
  m_ui.m_tcMessage->setCurrentIndex(1);
}

void FormMessageFiltersManager::loadSelectedAccount() {
  m_feedsModel->setRootItem(selectedAccount(), false, true);
  m_ui.m_treeFeeds->expandAll();
}

void FormMessageFiltersManager::loadFilterFeedAssignments() {}

void FormMessageFiltersManager::showFilter(MessageFilter* filter) {
  m_loadingFilter = true;

  if (filter == nullptr) {
    m_ui.m_txtTitle->clear();
    m_ui.m_txtScript->clear();
    m_ui.m_gbDetails->setEnabled(false);
  }
  else {
    m_ui.m_txtTitle->setText(filter->name());
    m_ui.m_txtScript->setPlainText(filter->script());
    m_ui.m_gbDetails->setEnabled(true);
  }

  // See message.
  m_ui.m_tcMessage->setCurrentIndex(0);
  m_loadingFilter = false;
}

void FormMessageFiltersManager::loadAccounts() {
  for (auto* acc : m_accounts) {
    m_ui.m_cmbAccounts->addItem(acc->icon(),
                                acc->title(),
                                QVariant::fromValue(acc));
  }
}

void FormMessageFiltersManager::beautifyScript() {
  QProcess proc_clang_format(this);

  proc_clang_format.setInputChannelMode(QProcess::InputChannelMode::ManagedInputChannel);
  proc_clang_format.setArguments({"--assume-filename=script.js", "--style=Chromium"});

#if defined (Q_OS_WIN)
  proc_clang_format.setProgram(qApp->applicationDirPath() + QDir::separator() +
                               QSL("clang-format") + QDir::separator() +
                               QSL("clang-format.exe"));
#else
  proc_clang_format.setProgram(QSL("clang-format"));
#endif

  if (!proc_clang_format.open()) {
    MessageBox::show(this, QMessageBox::Icon::Critical,
                     tr("Cannot find 'clang-format'"),
                     tr("Script was not beautified, because 'clang-format' tool was not found."));
    return;
  }

  proc_clang_format.write(m_ui.m_txtScript->toPlainText().toUtf8());
  proc_clang_format.closeWriteChannel();

  if (proc_clang_format.waitForFinished(3000)) {
    if (proc_clang_format.exitCode() == 0) {
      auto script = proc_clang_format.readAllStandardOutput();

      m_ui.m_txtScript->setPlainText(script);
    }
    else {
      auto err = proc_clang_format.readAllStandardError();

      MessageBox::show(this, QMessageBox::Icon::Critical,
                       tr("Error"),
                       tr("Script was not beautified, because 'clang-format' tool thrown error."),
                       QString(),
                       err);
    }
  }
  else {
    proc_clang_format.kill();
    MessageBox::show(this, QMessageBox::Icon::Critical,
                     tr("Beautifier was running for too long time"),
                     tr("Script was not beautified, is 'clang-format' installed?"));
  }
}

void FormMessageFiltersManager::initializeTestingMessage() {
  m_ui.m_cbSampleImportant->setChecked(true);
  m_ui.m_txtSampleUrl->setText(QSL("https://mynews.com/news/5"));
  m_ui.m_txtSampleTitle->setText(QSL("Year of Linux Desktop"));
  m_ui.m_txtSampleAuthor->setText(QSL("Napoleon Bonaparte"));
  m_ui.m_txtSampleContents->setPlainText(QSL("<p>Browsers usually insert quotation marks around the q element.</p>"
                                             "<p>WWF's goal is to: <q>Build a future where people live in harmony "
                                             "with nature.</q></p>"));
  m_ui.m_txtSampleCreatedOn->setText(QString::number(QDateTime::currentDateTimeUtc().toMSecsSinceEpoch()));
}

Message FormMessageFiltersManager::testingMessage() const {
  Message msg;

  msg.m_url = m_ui.m_txtSampleUrl->text();
  msg.m_title = m_ui.m_txtSampleTitle->text();
  msg.m_author = m_ui.m_txtSampleAuthor->text();
  msg.m_isRead = m_ui.m_cbSampleRead->isChecked();
  msg.m_isImportant = m_ui.m_cbSampleImportant->isChecked();
  msg.m_created = QDateTime::fromMSecsSinceEpoch(m_ui.m_txtSampleCreatedOn->text().toLongLong());
  msg.m_contents = m_ui.m_txtSampleContents->toPlainText();

  return msg;
}
