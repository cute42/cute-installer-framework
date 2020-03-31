/**************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Installer Framework.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
**************************************************************************/

#include "metadatajob.h"
#include "settings.h"
#include "init.h"
#include "../shared/commonfunctions.h"

#include <binarycontent.h>
#include <component.h>
#include <errors.h>
#include <fileutils.h>
#include <packagemanagercore.h>
#include <progresscoordinator.h>

#include <QLoggingCategory>
#include <QTest>

using namespace QInstaller;

class tst_CommandLineUpdate : public QObject
{
    Q_OBJECT

private:
    void setRepository(const QString &repository)
    {
        core->reset();
        core->cancelMetaInfoJob(); //Call cancel to reset metadata so that update repositories are fetched

        QSet<Repository> repoList;
        Repository repo = Repository::fromUserInput(repository);
        repoList.insert(repo);
        core->settings().setDefaultRepositories(repoList);
    }

private slots:
    void initTestCase()
    {
        core = new PackageManagerCore(BinaryContent::MagicInstallerMarker, QList<OperationBlob> ());
        QString appFilePath = QCoreApplication::applicationFilePath();
        core->setAllowedRunningProcesses(QStringList() << appFilePath);
        m_installDir = QInstaller::generateTemporaryFileName();
        QDir().mkpath(m_installDir);
        core->setValue(scTargetDir, m_installDir);
    }

    void testUpdatePackageSilently()
    {
        QInstaller::init(); //This will eat debug output
        setRepository(":///data/installPackagesRepository");
        core->installSelectedComponentsSilently(QStringList() << "componentA" << "componentB");
        VerifyInstaller::verifyInstallerResources(m_installDir, "componentA", "1.0.0content.txt");
        VerifyInstaller::verifyInstallerResources(m_installDir, "componentB", "1.0.0content.txt");
        VerifyInstaller::verifyFileExistence(m_installDir, QStringList() << "components.xml" << "installcontent.txt"
                           << "installcontentA.txt" << "installcontentE.txt" << "installcontentG.txt"
                           << "installcontentB.txt" << "installcontentD.txt");
        core->commitSessionOperations();

        setRepository(":///data/installPackagesRepositoryUpdate");
        core->updateComponentsSilently(QStringList() << "componentA");
        VerifyInstaller::verifyInstallerResources(m_installDir, "componentA", "2.0.0content.txt");
        VerifyInstaller::verifyInstallerResources(m_installDir, "componentB", "1.0.0content.txt");
        VerifyInstaller::verifyInstallerResourceFileDeletion(m_installDir, "componentA", "1.0.0content.txt");
        VerifyInstaller::verifyFileExistence(m_installDir, QStringList() << "components.xml" << "installcontentA_update.txt"
                           << "installcontentE.txt" << "installcontentG.txt"
                           << "installcontentB.txt" << "installcontentD.txt");
    }

    void testUpdateTwoPackageSilently()
    {
        QInstaller::init(); //This will eat debug output
        setRepository(":///data/installPackagesRepository");
        core->installSelectedComponentsSilently(QStringList() << "componentA" << "componentB" << "componentG");
        VerifyInstaller::verifyInstallerResources(m_installDir, "componentB", "1.0.0content.txt");
        VerifyInstaller::verifyInstallerResources(m_installDir, "componentG", "1.0.0content.txt");
        core->commitSessionOperations();

        setRepository(":///data/installPackagesRepositoryUpdate");
        core->updateComponentsSilently(QStringList() << "componentB" << "componentG");
        VerifyInstaller::verifyInstallerResources(m_installDir, "componentB", "2.0.0content.txt");
        VerifyInstaller::verifyInstallerResources(m_installDir, "componentG", "2.0.0content.txt");
        VerifyInstaller::verifyInstallerResourceFileDeletion(m_installDir, "componentB", "1.0.0content.txt");
        VerifyInstaller::verifyInstallerResourceFileDeletion(m_installDir, "componentG", "1.0.0content.txt");
    }

    void testUpdateAllPackagesSilently()
    {
        QInstaller::init(); //This will eat debug output
        setRepository(":///data/installPackagesRepository");
        core->installSelectedComponentsSilently(QStringList() << "componentA" << "componentB" << "componentG" << "componentF");
        VerifyInstaller::verifyInstallerResources(m_installDir, "componentF", "1.0.0content.txt");
        VerifyInstaller::verifyInstallerResources(m_installDir, "componentF.subcomponent1", "1.0.0content.txt");
        VerifyInstaller::verifyInstallerResources(m_installDir, "componentF.subcomponent1.subsubcomponent1", "1.0.0content.txt");
        VerifyInstaller::verifyInstallerResources(m_installDir, "componentF.subcomponent2", "1.0.0content.txt");
        core->commitSessionOperations();

        setRepository(":///data/installPackagesRepositoryUpdate");
        core->updateComponentsSilently(QStringList());
        VerifyInstaller::verifyInstallerResources(m_installDir, "componentF", "2.0.0content.txt");
        VerifyInstaller::verifyInstallerResources(m_installDir, "componentF.subcomponent2", "2.0.0content.txt");
        VerifyInstaller::verifyInstallerResourceFileDeletion(m_installDir, "componentF", "1.0.0content.txt");
        VerifyInstaller::verifyInstallerResourceFileDeletion(m_installDir, "componentF.subcomponent2", "1.0.0content.txt");
    }

    void cleanupTestCase()
    {
        QDir dir(m_installDir);
        QVERIFY(dir.removeRecursively());
        delete core;
    }

private:
    QString m_installDir;
    PackageManagerCore *core;
};


QTEST_MAIN(tst_CommandLineUpdate)

#include "tst_commandlineupdate.moc"
