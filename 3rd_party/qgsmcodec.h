
/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef GSM_CODEC
#define GSM_CODEC

#include <QString>
#include <QTextCodec>

class QGsmCodec : public QTextCodec
{
public:
    explicit QGsmCodec( bool noLoss=false );
    ~QGsmCodec();

    QByteArray name() const;
    int mibEnum() const;

    static char singleFromUnicode(QChar ch);
    static QChar singleToUnicode(char ch);

    static unsigned short twoByteFromUnicode(QChar ch);
    static QChar twoByteToUnicode(unsigned short ch);

    static QString fromGsm7BitEncodedtoUnicode(const char*);
    static QString fromUnicodeStringInHexToUnicode(const char*);

   static  bool stringHex( const QByteArray& ) ;

protected:
    QString convertToUnicode(const char *in, int length, ConverterState *state) const;
    QByteArray convertFromUnicode(const QChar *in, int length, ConverterState *state) const;

private:
    bool noLoss;
};

#endif
