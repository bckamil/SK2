from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtWidgets import QApplication, QMainWindow, QLineEdit

import threading
import socket


class Communication():
    def __init__(self):
        self.host = '127.0.0.1'
        self.port = 8085
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


class GameCommunication(Communication):
    def __init__(self):
        super(GameCommunication, self).__init__()
        self.game_status = None
        t2 = threading.Thread(target=self.listen)
        t2.start()

    def create_room(self):
        self.send_data(b'create\r\n')

    def join(self, id):
        command = 'join ' + str(id) + '\r\n'
        command = bytes(command, encoding='utf-8')

        return self.send_data(command)

    def ready(self):
        self.sock.send(b'ready\r\n')

    def listen(self):
        while True:
            self.game_status = self.sock.recv(1024).decode('utf-8')
            self.game_label.setText('')
            if self.game_status == 'Unlucky\n' or self.game_status == 'Wygrales\n' or self.game_status == 'Nie zyjesz\n':
                self.game_win_label.setText(self.game_status)
            else:
                self.game_label.setText(self.game_status)

    def send_letter(self, letter):
        command = 'send ' + letter + '\r\n'
        command = bytes(command, encoding='utf-8')

        self.send_data(command)

    def leave(self):
        self.send_data(b'leave\r\n')