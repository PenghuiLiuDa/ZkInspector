#include "googlesuggest.h"
#include <QLineEdit>
#include <QHeaderView>
//#include <QTimer>

GSuggestCompletion::GSuggestCompletion(QLineEdit* parent,QTreeWidget*  &widget) : QObject((QObject*)parent), editor(parent), ui_widget(widget)
{
	popup = new QTreeWidget;
	popup->setWindowFlags(Qt::Popup);
	popup->setFocusPolicy(Qt::NoFocus);
	popup->setFocusProxy((QWidget*)parent);
	popup->setMouseTracking(true);

	//设置treewidget当节点名字过长时可以水平滚动
	popup->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	popup->header()->setStretchLastSection(false);

	popup->setColumnCount(1);
	popup->setUniformRowHeights(true);
	popup->setRootIsDecorated(false);
	popup->setEditTriggers(QTreeWidget::NoEditTriggers);
	popup->setSelectionBehavior(QTreeWidget::SelectRows);
	popup->setFrameStyle(QFrame::Box | QFrame::Plain);
	popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	//popup->header()->hide();
	popup->setHeaderHidden(true);

	popup->installEventFilter(this);

	connect(popup, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
		SLOT(doneCompletion()));
	timer = new QTimer(this);
	timer->setSingleShot(true);
	timer->setInterval(50);    //50毫秒检测不到输入就输出
	connect(timer, SIGNAL(timeout()), SLOT(autoSuggest()));
	connect(editor, SIGNAL(textEdited(QString)), timer, SLOT(start()));
}

GSuggestCompletion::~GSuggestCompletion()
{
	delete popup;
}

bool GSuggestCompletion::eventFilter(QObject* obj, QEvent* ev)
{
	if (obj != popup)
		return false;

	if (ev->type() == QEvent::MouseButtonPress) {
		popup->hide();
		editor->setFocus();
		return true;
	}
	if (ev->type() == QEvent::KeyPress) {

		bool consumed = false;
		int key = static_cast<QKeyEvent*>(ev)->key();
		switch (key) {
		case Qt::Key_Enter:
		case Qt::Key_Return:
			doneCompletion();
			consumed = true;

		case Qt::Key_Escape:
			editor->setFocus();
			popup->hide();
			consumed = true;

		case Qt::Key_Up:
		case Qt::Key_Down:
		case Qt::Key_Home:
		case Qt::Key_End:
		case Qt::Key_PageUp:
		case Qt::Key_PageDown:
			break;

		default:
			editor->setFocus();
			editor->event(ev);
			popup->hide();
			break;
		}

		return consumed;
	}
	return false;
}

void GSuggestCompletion::showCompletion(const QStringList& choices)
{

	if (choices.isEmpty())
		return;

	const QPalette& pal = editor->palette();
	QColor color = pal.color(QPalette::Disabled, QPalette::WindowText);

	popup->setUpdatesEnabled(false);
	popup->clear();
	for (int i = 0; i < choices.count(); ++i) {
		QTreeWidgetItem* item;
		item = new QTreeWidgetItem(popup);
		item->setText(0, choices[i]);
		item->setTextAlignment(1, Qt::AlignRight);
		item->setTextColor(1, color);
	}
	popup->setCurrentItem(popup->topLevelItem(0));
	popup->resizeColumnToContents(0);
	popup->resizeColumnToContents(1);
	popup->adjustSize();
	popup->setUpdatesEnabled(true);

	int h = popup->sizeHintForRow(0) * qMin(10, choices.count()) + 3;
	popup->resize(editor->width(), h);

	popup->move(editor->mapToGlobal(QPoint(0, editor->height())));
	popup->setFocus();
	popup->show();
}

void GSuggestCompletion::doneCompletion()
{
	timer->stop();
	popup->hide();
	editor->setFocus();
	QTreeWidgetItem* item = popup->currentItem();
	if (item) {
		int index = popup->indexOfTopLevelItem(item);
		qDebug() << index << endl;
		QTreeWidgetItem* item_new = search_result[index];
		//ui.tree_widget->scrollToItem(item);
		ui_widget->setCurrentItem(item_new);
	}
}

void GSuggestCompletion::autoSuggest()
{
	QString str = editor->text();
	//QString url = QString(GSUGGEST_URL).arg(str);
	search_result = ui_widget->findItems(str, Qt::MatchContains | Qt::MatchRecursive);
	QStringList suggest;
	for (int i = 0; i < search_result.size(); i++)
	{
		suggest << search_result[i]->text(0);
	}
	showCompletion(suggest);
}

void GSuggestCompletion::preventSuggest()
{
	timer->stop();
}

