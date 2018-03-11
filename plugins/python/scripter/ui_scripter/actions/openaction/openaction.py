"""
Copyright (c) 2017 Eliakin Costa <eliakim170@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
"""
from PyQt5.QtWidgets import QAction, QFileDialog, QMessageBox
from PyQt5.QtGui import QKeySequence, QIcon
from PyQt5.QtCore import Qt


class OpenAction(QAction):

    def __init__(self, scripter, parent=None):
        super(OpenAction, self).__init__(parent)
        self.scripter = scripter

        self.triggered.connect(self.open)

        self.setText('Open')
        self.setObjectName('open')
        self.setShortcut(QKeySequence(Qt.CTRL + Qt.Key_O))
        self.setIcon(Krita.instance().icon("document-open"))

    @property
    def parents(self):
        return ('File', 'toolBar')

    def open(self):
        dialog = QFileDialog(self.scripter.uicontroller.mainWidget)
        dialog.setNameFilter('Python files (*.py)')

        if dialog.exec():
            try:
                selectedFile = dialog.selectedFiles()[0]
                fileExtension = selectedFile.rsplit('.', maxsplit=1)[1]

                if fileExtension == 'py':
                    document = self.scripter.documentcontroller.openDocument(selectedFile)
                    self.scripter.uicontroller.setDocumentEditor(document)
                    self.scripter.uicontroller.setStatusBar(document.filePath)
                    print("open is run")
            except Exception:
                QMessageBox.information(self.scripter.uicontroller.mainWidget,
                                        'Invalid File',
                                        'Open files with .py extension')
