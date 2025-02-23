/** @file
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef UI_QT_ADDITIONAL_TOOLBAR_H_
#define UI_QT_ADDITIONAL_TOOLBAR_H_

#include <epan/plugin_if.h>

#include <QToolBar>
#include <QWidgetAction>

/* Class for all display widgets.
 *
 * Inherits QWidgetAction, otherwise the extension popup might not work for the toolbar
 */
class AdditionalToolbarWidgetAction: public QWidgetAction
{
    Q_OBJECT

public:

    AdditionalToolbarWidgetAction(QObject * parent = 0);
    AdditionalToolbarWidgetAction(ext_toolbar_t * item, QObject * parent = 0);
    AdditionalToolbarWidgetAction(const AdditionalToolbarWidgetAction & copy_object);
    ~AdditionalToolbarWidgetAction();

protected:
    virtual QWidget * createWidget(QWidget * parent);

    static const char * propertyName;

private:

    ext_toolbar_t * toolbar_item;

    QWidget * createButton(ext_toolbar_t * item, QWidget * parent);
    QWidget * createBoolean(ext_toolbar_t * item, QWidget * parent);
    QWidget * createTextEditor(ext_toolbar_t * item, QWidget * parent);
    QWidget * createSelector(ext_toolbar_t * item, QWidget * parent);

    QWidget * createLabelFrame(ext_toolbar_t * item, QWidget * parent);

    ext_toolbar_t * extractToolbarItemFromObject(QObject *);

private slots:
    void onButtonClicked();
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    void onCheckBoxChecked(Qt::CheckState);
#else
    void onCheckBoxChecked(int);
#endif
    void sendTextToCallback();
    void onSelectionInWidgetChanged(int idx);

    void captureActive(int);
};

class AdditionalToolBar: public QToolBar
{
    Q_OBJECT

public:
    AdditionalToolBar(ext_toolbar_t * toolbar, QWidget * parent = 0);
    virtual ~AdditionalToolBar();

    static AdditionalToolBar * create(QWidget * parent, ext_toolbar_t * toolbar);

    QString menuName();

private:
    ext_toolbar_t * toolbar;
};

#endif /* UI_QT_ADDITIONAL_TOOLBAR_H_ */
