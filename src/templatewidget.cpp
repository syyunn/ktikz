/***************************************************************************
 *   Copyright (C) 2008 by Glad Deschrijver                                *
 *   Glad.Deschrijver@UGent.be                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QApplication>
#include <QComboBox>
#include <QLineEdit>
#include <QFileDialog>
#include <QKeyEvent>
#include <QProcess>
#include <QSettings>

#include "lineedit.h"
#include "templatewidget.h"

TemplateWidget::TemplateWidget(QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);
	ui.templateCombo->setLineEdit(new LineEdit(this));
	ui.templateCombo->setMinimumContentsLength(20);

	connect(ui.templateChooseButton, SIGNAL(clicked()),
	        this, SLOT(setTemplateFile()));
	connect(ui.templateEditButton, SIGNAL(clicked()),
	        this, SLOT(editTemplateFile()));
	connect(ui.templateCombo->lineEdit(), SIGNAL(textChanged(const QString&)),
	        this, SIGNAL(fileNameChanged(const QString&)));

	readRecentTemplates();
}

TemplateWidget::~TemplateWidget()
{
	saveRecentTemplates();
}

void TemplateWidget::readRecentTemplates()
{
	QSettings settings;
	ui.templateCombo->setMaxCount(settings.value("TemplateRecentNumber", 5).toInt());
	ui.templateCombo->addItems(settings.value("TemplateRecent").toStringList());
}

void TemplateWidget::saveRecentTemplates()
{
	QSettings settings;
	QStringList recentTemplates;
	for (int i = 0; i < ui.templateCombo->count(); ++i)
		recentTemplates << ui.templateCombo->itemText(i);
	settings.setValue("TemplateRecent", recentTemplates);
}

void TemplateWidget::setFileName(const QString &fileName)
{
	int index = ui.templateCombo->findText(fileName);
	if (index >= 0) // then remove item in order to re-add it at the top
		ui.templateCombo->removeItem(index);
	ui.templateCombo->insertItem(0, fileName);
	ui.templateCombo->lineEdit()->setText(fileName);
}

void TemplateWidget::setReplaceText(const QString &replace)
{
	QString replaceText = replace;
	replaceText.replace("&", "&amp;");
	replaceText.replace("<", "&lt;");
	replaceText.replace(">", "&gt;");
	QString templateDescription(tr("<p>The template contains the code "
	    "of a complete LaTeX document in which the TikZ picture will be "
	    "included and which will be typesetted to produce the preview "
	    "image.  The string %1 in the template will be replaced by the "
	    "TikZ code.</p>").arg(replaceText));
	ui.templateCombo->setWhatsThis(tr("<p>Give the file name of the LaTeX "
	    "template.  If this input field is empty or contains an invalid "
	    "file name, an internal default template will be used.</p>")
	    + templateDescription);
	ui.templateLabel->setWhatsThis(ui.templateCombo->whatsThis());
	ui.templateEditButton->setWhatsThis(tr("<p>Edit this template with "
	    "an external editor specified in the \"Configure\" dialog.</p>")
	    + templateDescription);
}

void TemplateWidget::setEditor(const QString &editor)
{
	m_editor = editor;
}

QString TemplateWidget::fileName() const
{
	return ui.templateCombo->currentText();
}

void TemplateWidget::setTemplateFile()
{
	const QString fileName = QFileDialog::getOpenFileName(this,
	    tr("Select a template file"), ui.templateCombo->currentText(),
	    QString("%1 (*.pgs *.tex);;%2 (*.*)")
	    .arg(tr("KTikZ template files")).arg(tr("All files")));
	if (!fileName.isEmpty())
		setFileName(fileName);
}

void TemplateWidget::editTemplateFile()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	QStringList editorArguments;
	editorArguments << ui.templateCombo->currentText();

	QProcess process;
	process.startDetached(m_editor, editorArguments);

	QApplication::restoreOverrideCursor();
}

void TemplateWidget::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Return)
		setFileName(ui.templateCombo->currentText());
	if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Return)
		emit focusEditor();
	QWidget::keyPressEvent(event);
}