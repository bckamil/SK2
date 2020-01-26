from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtWidgets import QApplication, QMainWindow, QLineEdit

import sys

from game import GameCommunication


class Main(QMainWindow, GameCommunication):    
    def __init__(self, parent=None):
        super(Main, self).__init__(parent)
        self.setupUi()

    def setupUi(self):
        self.setObjectName("self")
        self.resize(600, 400)

        self.QtStack = QtWidgets.QStackedLayout()

        self.stack1 = QtWidgets.QWidget()
        self.stack2 = QtWidgets.QWidget()

        self.games()
        self.game()

        self.QtStack.addWidget(self.stack1)
        self.QtStack.addWidget(self.stack2)

    def games(self):
        self.stack1.resize(600, 400)

        self.textbox = QLineEdit(self.stack1)
        self.textbox.move(20, 20)
        self.textbox.resize(280, 40)
        
        self.button_join = QtWidgets.QPushButton('Join', self.stack1)
        self.button_join.move(320, 20)
        self.button_join.resize(60, 40)

        self.button_create = QtWidgets.QPushButton('Create', self.stack1)
        self.button_create.move(400, 20)
        self.button_create.resize(60, 40)

        self.button_join.clicked.connect(self.click_join)
        self.button_create.clicked.connect(self.click_create)

    def click_join(self):
        id = self.textbox.text()
        if id != '':
            self.join(id)
            self.QtStack.setCurrentIndex(1)

    def click_create(self):
        self.create_room()
        self.QtStack.setCurrentIndex(1)

    def game(self):
        self.stack2.resize(600, 400)

        letters_1 = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h']
        letters_2 = ['i', 'j', 'k', 'l', 'm', 'n', 'o', 'p']
        letters_3 = ['r', 's', 't', 'u', 'w', 'x', 'y', 'z']

        for i, letter in enumerate(letters_1):
            button = QtWidgets.QPushButton(letter, self.stack2)
            button.setGeometry(QtCore.QRect(i*40 + i * 20 + 40, 160, 40, 40))
            button.clicked.connect(self.make_letter(letter))

        for i, letter in enumerate(letters_2):
            button = QtWidgets.QPushButton(letter, self.stack2)
            button.setGeometry(QtCore.QRect(i*40 + i * 20 + 40, 220, 40, 40))
            button.clicked.connect(self.make_letter(letter))

        for i, letter in enumerate(letters_3):
            button = QtWidgets.QPushButton(letter, self.stack2)
            button.setGeometry(QtCore.QRect(i*40 + i * 20 + 40, 280, 40, 40))
            button.clicked.connect(self.make_letter(letter))

        self.game_label = QtWidgets.QLabel(str(self.game_status), self.stack2)
        self.game_label.setGeometry(QtCore.QRect(60, 60, 300, 40))

        self.game_win_label = QtWidgets.QLabel(str(''), self.stack2)
        self.game_win_label.setGeometry(QtCore.QRect(60, 105, 300, 40))

        self.ready_button = QtWidgets.QPushButton('Ready', self.stack2)
        self.ready_button.setGeometry(QtCore.QRect(40, 340, 60, 40))
        self.ready_button.clicked.connect(self.set_ready)

        self.ready_button = QtWidgets.QPushButton('Leave', self.stack2)
        self.ready_button.setGeometry(QtCore.QRect(120, 340, 60, 40))
        self.ready_button.clicked.connect(self.set_leave)

    def set_ready(self):
        self.ready()
        self.game_win_label.setText('')

    def set_leave(self):
        self.leave()
        self.game_win_label.setText('')
        self.QtStack.setCurrentIndex(0)

    def make_letter(self, letter):
        def send_l():
            self.send_letter(letter)
        return send_l


def main():
    app = QApplication(sys.argv)
    showMain = Main()
    sys.exit(app.exec_())


if __name__ == '__main__':
    main()
