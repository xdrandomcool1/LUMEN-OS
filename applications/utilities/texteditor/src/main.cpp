// SPDX-License-Identifier: GPL-3.0-or-later
#include "../../../common-main.h"
#include <QUrl>
static QVariantMap initial(const QStringList &args)
{
    return {{QStringLiteral("openFile"), args.isEmpty() ? QString() : QUrl::fromUserInput(args.first(), QString(), QUrl::AssumeLocalFile).toString()}};
}
int main(int argc, char **argv) { return Lumen::runApp(argc, argv, "lumen-texteditor", "Text Editor", "Lumen.TextEditor", initial); }
