/* Copyright (C) 2007 Jan Kundrát <jkt@gentoo.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "Imap/Message.h"

namespace Imap {
namespace Message {

QList<MailAddress> Envelope::getListOfAddresses( const QVariant& in, const QByteArray& line, const int start )
{
    if ( in.type() == QVariant::ByteArray ) {
        if ( ! in.toByteArray().isNull() )
            throw UnexpectedHere( line, start );
    } else if ( in.type() != QVariant::List )
        throw ParseError( line, start );

    QVariantList list = in.toList();
    QList<MailAddress> res;
    for ( QVariantList::const_iterator it = list.begin(); it != list.end(); ++it ) {
        if ( it->type() != QVariant::List )
            throw UnexpectedHere( line, start ); // FIXME: wrong offset
        res.append( MailAddress( it->toList(), line, start ) );
    }
    return res;
}

MailAddress::MailAddress( const QVariantList& input, const QByteArray& line, const int start )
{
    // FIXME: all offsets are wrong here
    // FIXME: decode strings from various RFC2822 encodings
    if ( input.size() != 4 )
        throw ParseError( line, start );

    if ( input[0].type() != QVariant::ByteArray )
        throw UnexpectedHere( line, start );
    if ( input[1].type() != QVariant::ByteArray )
        throw UnexpectedHere( line, start );
    if ( input[2].type() != QVariant::ByteArray )
        throw UnexpectedHere( line, start );
    if ( input[3].type() != QVariant::ByteArray )
        throw UnexpectedHere( line, start );

    name = input[0].toByteArray();
    adl = input[1].toByteArray();
    mailbox = input[2].toByteArray();
    host = input[3].toByteArray();
}

Envelope Envelope::fromList( const QVariantList& items, const QByteArray& line, const int start )
{
    if ( items.size() != 10 )
        throw ParseError( line, start ); // FIXME: wrong offset

    // date
    QDateTime date;
    if ( items[0].type() == QVariant::ByteArray ) {
        QByteArray dateStr = items[0].toByteArray();
        date = LowLevelParser::parseRFC2822DateTime( dateStr );
    }
    // Otherwise it's "invalid", null.

    QByteArray subject = items[1].toByteArray(); // FIXME: decode

    QList<MailAddress> from, sender, replyTo, to, cc, bcc;
    from = Envelope::getListOfAddresses( items[2], line, start );
    sender = Envelope::getListOfAddresses( items[3], line, start );
    replyTo = Envelope::getListOfAddresses( items[4], line, start );
    to = Envelope::getListOfAddresses( items[5], line, start );
    cc = Envelope::getListOfAddresses( items[6], line, start );
    bcc = Envelope::getListOfAddresses( items[7], line, start );

    if ( items[8].type() != QVariant::ByteArray )
        throw UnexpectedHere( line, start );
    QByteArray inReplyTo = items[8].toByteArray();

    if ( items[9].type() != QVariant::ByteArray )
        throw UnexpectedHere( line, start );
    QByteArray messageId = items[9].toByteArray();

    return Envelope( date, subject, from, sender, replyTo, to, cc, bcc, inReplyTo, messageId );
}

bool TextMessage::eq( const AbstractData& other ) const
{
    // FIXME
    return false;
}

QTextStream& TextMessage::dump( QTextStream& s ) const
{
    return s;
}

bool MsgMessage::eq( const AbstractData& other ) const
{
    // FIXME
    return false;
}

QTextStream& MsgMessage::dump( QTextStream& s ) const
{
    return s;
}

bool BasicMessage::eq( const AbstractData& other ) const
{
    // FIXME
    return false;
}

QTextStream& BasicMessage::dump( QTextStream& s ) const
{
    return s;
}

bool MultiMessage::eq( const AbstractData& other ) const
{
    // FIXME
    return false;
}

QTextStream& MultiMessage::dump( QTextStream& s ) const
{
    return s;
}

AbstractMessage::bodyFldParam_t AbstractMessage::makeBodyFldParam( const QVariant& input, const QByteArray& line, const int start )
{
    if ( input.type() != QVariant::List )
        throw UnexpectedHere( line, start ); // body-fld-param: not a list
    bodyFldParam_t map;
    QVariantList list = input.toList();
    if ( list.size() % 2 )
        throw UnexpectedHere( line, start ); // body-fld-param: wrong number of entries
    for ( int j = 0; j < list.size(); j += 2 )
        if ( list[j].type() != QVariant::ByteArray || list[j+1].type() != QVariant::ByteArray )
            throw UnexpectedHere( line, start ); // body-fld-param: string not found
        else
            map[ list[j].toByteArray() ] = list[j+1].toByteArray();
    return map;
}

AbstractMessage::bodyFldDsp_t AbstractMessage::makeBodyFldDsp( const QVariant& input, const QByteArray& line, const int start )
{
    if ( input.type() != QVariant::List )
        throw UnexpectedHere( line, start ); // body-fld-dsp: not a list
    QVariantList list = input.toList();
    bodyFldDsp_t res;
    if ( list.size() != 2 )
        throw ParseError( line, start ); // body-fld-dsp: wrong number of entries in the list
    if ( list[0].type() != QVariant::ByteArray )
        throw UnexpectedHere( line, start ); // body-fld-dsp: first item is not a string
    res.first = list[0].toByteArray();
    if ( list[1].type() != QVariant::List )
        throw UnexpectedHere( line, start ); // body-fld-dsp: body-fld-param not recognized
    res.second = makeBodyFldParam( list[1].toList(), line, start );
    return res;
}

QList<QByteArray> AbstractMessage::makeBodyFldLang( const QVariant& input, const QByteArray& line, const int start )
{
    QList<QByteArray> res;
    if ( input.type() == QVariant::ByteArray ) {
        res << input.toByteArray();
    } else if ( input.type() == QVariant::List ) {
        QVariantList list = input.toList();
        for ( QVariantList::const_iterator it = list.begin(); it != list.end(); ++it )
            if ( it->type() != QVariant::ByteArray )
                throw UnexpectedHere( line, start ); // body-fld-lang has wrong structure
            else
                res << it->toByteArray();
    } else
        throw UnexpectedHere( line, start ); // body-fld-lang not found
    return res;
}

std::tr1::shared_ptr<AbstractMessage> AbstractMessage::fromList( const QVariantList& items, const QByteArray& line, const int start )
{
    if ( items.size() < 3 )
        throw NoData( line, start );

    if ( items[0].type() == QVariant::ByteArray ) {
        // it's a single-part message, hurray
        if ( items.size() < 7 )
            throw NoData( line, start );

        int i = 0;
        QString mediaType = items[i].toString().toLower();
        ++i;
        QString mediaSubType = items[i].toString().toLower();
        ++i;

        bodyFldParam_t bodyFldParam = makeBodyFldParam( items[i], line, start );
        ++i;

        if ( items[i].type() != QVariant::ByteArray )
            throw UnexpectedHere( line, start ); // body-fld-id not recognized
        QByteArray bodyFldId = items[i].toByteArray();
        ++i;

        if ( items[i].type() != QVariant::ByteArray )
            throw UnexpectedHere( line, start ); // body-fld-desc not recognized
        QByteArray bodyFldDesc = items[i].toByteArray();
        ++i;

        if ( items[i].type() != QVariant::ByteArray )
            throw UnexpectedHere( line, start ); // body-fld-enc not recognized
        QByteArray bodyFldEnc = items[i].toByteArray();
        ++i;

        if ( items[i].type() != QVariant::UInt )
            throw UnexpectedHere( line, start ); // body-fld-octets not recognized
        uint bodyFldOctets = items[i].toUInt();
        ++i;

        uint bodyFldLines = 0;
        Envelope envelope;
        std::tr1::shared_ptr<AbstractMessage> body;

        enum { MESSAGE, TEXT, BASIC} kind;

        if ( mediaType == "message" && mediaSubType == "rfc822" ) {
            // extract envelope, body, body-fld-lines

            if ( items.size() < 10 )
                throw NoData( line, start ); // too few fields for a Message-message

            kind = MESSAGE;
            if ( items[i].type() != QVariant::List )
                throw UnexpectedHere( line, start ); // envelope not recognized
            envelope = Envelope::fromList( items[i].toList(), line, start );
            ++i;

            if ( items[i].type() != QVariant::List )
                throw UnexpectedHere( line, start ); // body not recognized
            body = AbstractMessage::fromList( items[i].toList(), line, start );
            ++i;

            if ( items[i].type() != QVariant::UInt )
                throw UnexpectedHere( line, start ); // body-fld-lines not found
            bodyFldLines = items[i].toUInt();
            ++i;

        } else if ( mediaType == "text" ) {
            // extract body-fld-lines

            kind = TEXT;
            if ( items[i].type() != QVariant::UInt )
                throw UnexpectedHere( line, start ); // body-fld-lines not found
            bodyFldLines = items[i].toUInt();
            ++i;

        } else {
            // don't extract anything as we're done here
            kind = BASIC;
        }

        // extract body-ext-1part

        // body-fld-md5
        QByteArray bodyFldMd5;
        if ( i < items.size() ) {
            if ( items[i].type() != QVariant::ByteArray )
                throw UnexpectedHere( line, start ); // body-fld-md5 not found
            bodyFldMd5 = items[i].toByteArray();
            ++i;
        }

        // body-fld-dsp
        bodyFldDsp_t bodyFldDsp;
        if ( i < items.size() ) {
            bodyFldDsp = makeBodyFldDsp( items[i], line, start );
            ++i;
        }

        // body-fld-lang
        QList<QByteArray> bodyFldLang;
        if ( i < items.size() ) {
            bodyFldLang = makeBodyFldLang( items[i], line, start );
            ++i;
        }

        // body-fld-loc
        QByteArray bodyFldLoc;
        if ( i < items.size() ) {
            if ( items[i].type() != QVariant::ByteArray )
                throw UnexpectedHere( line, start ); // body-fld-loc not found
            bodyFldLoc = items[i].toByteArray();
            ++i;
        }

        // body-extension
        QVariant bodyExtension;
        if ( i < items.size() ) {
            if ( i == items.size() - 1 ) {
                bodyExtension = items[i];
                ++i;
            } else {
                QVariantList list;
                for ( ; i < items.size(); ++i )
                    list << items[i];
                bodyExtension = list;
            }
        }

        switch ( kind ) {
            case MESSAGE:
                return std::tr1::shared_ptr<AbstractMessage>(
                    new MsgMessage( mediaType, mediaSubType, bodyFldParam,
                        bodyFldId, bodyFldDesc, bodyFldEnc, bodyFldOctets,
                        bodyFldMd5, bodyFldDsp, bodyFldLang, bodyFldLoc,
                        bodyExtension, envelope, body, bodyFldLines )
                    );
            case TEXT:
                return std::tr1::shared_ptr<AbstractMessage>(
                    new TextMessage( mediaType, mediaSubType, bodyFldParam,
                        bodyFldId, bodyFldDesc, bodyFldEnc, bodyFldOctets,
                        bodyFldMd5, bodyFldDsp, bodyFldLang, bodyFldLoc,
                        bodyExtension, bodyFldLines )
                    );
            case BASIC:
            default:
                return std::tr1::shared_ptr<AbstractMessage>(
                    new BasicMessage( mediaType, mediaSubType, bodyFldParam,
                        bodyFldId, bodyFldDesc, bodyFldEnc, bodyFldOctets,
                        bodyFldMd5, bodyFldDsp, bodyFldLang, bodyFldLoc,
                        bodyExtension )
                    );
        }

    } else if ( items[0].type() == QVariant::List ) {

        if ( items.size() < 2 )
            throw ParseError( line, start ); // body-type-mpart: structure should be "body* string"

        int i = 0;

        QList<std::tr1::shared_ptr<AbstractMessage> > bodies;
        while ( items[i].type() == QVariant::List) {
            bodies << fromList( items[i].toList(), line, start );
            ++i;
        }

        if ( items[i].type() != QVariant::ByteArray )
            throw UnexpectedHere( line, start ); // body-type-mpart: media-subtype not recognized
        QString mediaSubType = items[i].toString();
        ++i;

        // body-ext-mpart

        // body-fld-param
        bodyFldParam_t bodyFldParam;
        if ( i < items.size() ) {
            bodyFldParam = makeBodyFldParam( items[i], line, start );
            ++i;
        }

        // body-fld-dsp
        bodyFldDsp_t bodyFldDsp;
        if ( i < items.size() ) {
            bodyFldDsp = makeBodyFldDsp( items[i], line, start );
            ++i;
        }

        // body-fld-lang
        QList<QByteArray> bodyFldLang;
        if ( i < items.size() ) {
            bodyFldLang = makeBodyFldLang( items[i], line, start );
            ++i;
        }

        // body-fld-loc
        QByteArray bodyFldLoc;
        if ( i < items.size() ) {
            if ( items[i].type() != QVariant::ByteArray )
                throw UnexpectedHere( line, start ); // body-fld-loc not found
            bodyFldLoc = items[i].toByteArray();
            ++i;
        }

        // body-extension
        QVariant bodyExtension;
        if ( i < items.size() ) {
            if ( i == items.size() - 1 ) {
                bodyExtension = items[i];
                ++i;
            } else {
                QVariantList list;
                for ( ; i < items.size(); ++i )
                    list << items[i];
                bodyExtension = list;
            }
        }

        return std::tr1::shared_ptr<AbstractMessage>(
                new MultiMessage( bodies, mediaSubType, bodyFldParam,
                    bodyFldDsp, bodyFldLang, bodyFldLoc, bodyExtension ) );
    } else {
        throw UnexpectedHere( line, start );
    }
}

QTextStream& operator<<( QTextStream& stream, const MailAddress& address )
{
    stream << '"' << address.name << "\" <";
    if ( !address.host.isNull() )
        stream << address.mailbox << '@' << address.host;
    else
        stream << address.mailbox;
    stream << '>';
    return stream;
}

QTextStream& operator<<( QTextStream& stream, const QList<MailAddress>& address )
{
    stream << "[ ";
    for ( QList<MailAddress>::const_iterator it = address.begin(); it != address.end(); ++it )
        stream << *it << ", ";
    return stream << " ]";
}

QTextStream& operator<<( QTextStream& stream, const Envelope& e )
{
    return stream << "Date: " << e.date.toString() << "\nSubject: " << e.subject << "\nFrom: " <<
        e.from << "\nSender: " << e.sender << "\nReply-To: " << 
        e.replyTo << "\nTo: " << e.to << "\nCc: " << e.cc << "\nBcc: " <<
        e.bcc << "\nIn-Reply-To: " << e.inReplyTo << "\nMessage-Id: " << e.messageId <<
        "\n";
}

bool operator==( const Envelope& a, const Envelope& b )
{
    return a.date == b.date && a.date == b.date && a.subject == b.subject &&
        a.from == b.from && a.sender == b.sender && a.replyTo == b.replyTo &&
        a.to == b.to && a.cc == b.cc && a.bcc == b.bcc &&
        a.inReplyTo == b.inReplyTo && a.messageId == b.messageId;
}

bool operator==( const MailAddress& a, const MailAddress& b )
{
    return a.name == b.name && a.adl == b.adl && a.mailbox == b.mailbox && a.host == b.host;
}



}
}
