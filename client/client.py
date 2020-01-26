from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtWidgets import QApplication, QMainWindow, QLineEdit

import sys
import threading

import socket


class Communication():
    def __init__(self):
        self.host = '127.0.0.1'
        self.port = 8082
        self.sock = self.connect(self.host, self.port)

    def connect(self, host, port):
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((host, port))

            return sock
        except:
            print('Connection error')
            sys.exit()

    def send_data(self, data):
        try:
            self.sock.send(data)
        except:
            msg = QtWidgets.QMessageBox()
            msg.setIcon(QtWidgets.QMessageBox.Information)

            msg.setText("Connection error")
            msg.setInformativeText("Please close your app")
            msg.setWindowTitle("Connection error")
            msg.setStandardButtons(QtWidgets.QMessageBox.Ok)

            retval = msg.exec_()
        
        return self.sock.recv(1024)


class GameCommunication(Communication):
    def __init__(self):
        super(GameCommunication, self).__init__()
        self.game_status = None

    def create_room(self):
        return self.send_data(b'create\r\n')

    def join(self, id):
        command = 'join ' + str(id) + '\r\n'
        command = bytes(command, encoding='utf-8')

        return self.send_data(command)

    def ready(self):
        self.sock.send(b'ready\r\n')
        #t2 = threading.Thread(target=self.listen)
        #t2.start()

    def listen(self):
        self.game_status = self.sock.recv(1024).decode('utf-8')

    def send_letter(self, letter):
        command = 'send ' + letter + '\r\n'
        command = bytes(command, encoding='utf-8')

        return self.send_data(command)

    def leave(self):
        self.send_data(b'leave\r\n')


class Main(QMainWindow, GameCommunication):    
    def __init__(self, parent=None):
        super(Main, self).__init__(parent)
        self.game_id = 0
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
            self.game_id = self.join(id).decode("utf-8")

            self.QtStack.setCurrentIndex(1)

    def click_create(self):
        self.game_id = self.create_room().decode("utf-8")
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

        self.id_label = QtWidgets.QLabel('Id: ' + str(self.game_id), self.stack2)
        self.id_label.setGeometry(QtCore.QRect(40, 40, 40, 40))

        self.game_label = QtWidgets.QLabel(str(self.game_status), self.stack2)

        self.ready_button = QtWidgets.QPushButton('Ready', self.stack2)
        self.ready_button.setGeometry(QtCore.QRect(40, 340, 60, 40))
        self.ready_button.clicked.connect(self.set_ready)

        self.ready_button = QtWidgets.QPushButton('Leave', self.stack2)
        self.ready_button.setGeometry(QtCore.QRect(120, 340, 60, 40))
        self.ready_button.clicked.connect(self.set_leave)

    def set_ready(self):
        self.ready()

    def set_leave(self):
        self.leave()
        self.QtStack.setCurrentIndex(0)

    def make_letter(self, letter):
        def send_l():
            command = 'send ' + letter + '\r\n'
            command = bytes(command, encoding='utf-8')

            self.send_data(command)
            self.game_status = self.sock.recv(1024).decode('utf-8')
            print(self.game_status)
            # self.game_status = self.send_letter(letter).decode('utf-8')
        return send_l

if __name__ == '__main__':
    app = QApplication(sys.argv)
    showMain = Main()
    sys.exit(app.exec_())
