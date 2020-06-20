#ifndef GOOGLESUGGEST_H
#define GOOGLESUGGEST_H

#include <QtGui>
//#include <QtNetwork>
#include <QObject>
#include <QTreeWidget>

class QLineEdit;
class QNetworkReply;
class QTimer;
class QTreeWidget;

class GSuggestCompletion : public QObject
{
	Q_OBJECT

public:
	GSuggestCompletion(QLineEdit* parent, QTreeWidget*  &widget);
	~GSuggestCompletion();
	bool eventFilter(QObject* obj, QEvent* ev);
	void showCompletion(const QStringList& choices);

public slots:

	void doneCompletion();
	void preventSuggest();
	void autoSuggest();
	// void handleNetworkData(QNetworkReply* networkReply);

private:
	QLineEdit* editor;
	QTreeWidget* popup;
	QTreeWidget* ui_widget;
	QTimer* timer;
	QList<QTreeWidgetItem* > search_result;

	// QNetworkAccessManager networkManager;
};
#endif // GOOGLESUGGEST_H
