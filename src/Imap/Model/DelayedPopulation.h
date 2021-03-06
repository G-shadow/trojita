/* Copyright (C) 2006 - 2013 Jan Kundrát <jkt@flaska.net>

   This file is part of the Trojita Qt IMAP e-mail client,
   http://trojita.flaska.net/

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef IMAP_MODEL_DELAYEDPOPULATION_H
#define IMAP_MODEL_DELAYEDPOPULATION_H

#include <QObject>
#include <QPersistentModelIndex>

namespace Imap
{

namespace Mailbox
{

class Model;

/** @short Helper for filing child mailboxes outside of MVC slots

There's a problem with TreeItemMailbox' handling of fetch(). The fetch() could easily get
called from something like rowCount(), and the attached proxy models or views or whatever
would get etremely confused when model emits and signals about new arrivals etc from inside
rowCount().

This class therefore acts as a safe wrapper delaying the actualy calls to beginInsertRows()
etc until the event loop is re-entered.

This is similar to former MailboxUpdater, except that that particular class was not safe with
regard to TreeItem lifetime. This one is.

This class will automatically delete itself when it's done. The intended usage is create-and-forget
like this:

    new DelayedAskForChildrenOfMailbox(model, mailbox);

 */
class DelayedAskForChildrenOfMailbox : public QObject
{
    Q_OBJECT
public:
    typedef enum {
        CACHE_NORMAL,
        CACHE_FORCE_RELOAD
    } CacheMode;

    DelayedAskForChildrenOfMailbox(Model *model, const QModelIndex &mailbox, const CacheMode cacheMode);
private slots:
    /** @short Call Model's askForChildrenOfMailbox() */
    void askNow();
private:
    /** @short The Model */
    Model *m_model;
    /** @short Mailbox which wants to have its children updated */
    QPersistentModelIndex m_mailbox;
    /** @short True if we're to ask for top-level mailboxes

    This is a special case because invalid parent means top-level mailbox. That could fool our
    logic of checking the persistent index for validity and could lead either to ignored requests
    or to infinite recursion.
    */
    bool m_topLevel;

    CacheMode m_cacheMode;
};

/** @short Helper for filing messages into a TreeItemMsgList outside of MVC slots

@see DelayedAskForChildrenOfMailbox
*/
class DelayedAskForMessagesInMailbox : public QObject
{
    Q_OBJECT
public:
    DelayedAskForMessagesInMailbox(Model *model, const QModelIndex &list);
private slots:
    /** @short Call Model's askForMessagesInMailbox() */
    void askNow();
private:
    /** @short The Model */
    Model *m_model;
    /** @short Mailbox which wants to have its children updated */
    QPersistentModelIndex m_list;
};

}

}

#endif // IMAP_MODEL_DELAYEDPOPULATION_H
