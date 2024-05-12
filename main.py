from PyQt6.QtWidgets import (
    QApplication, QDialog, QPushButton, QHBoxLayout, QMessageBox
)

import sys

def make_push_button(text, action):
    button = QPushButton(text)
    button.clicked.connect(action)
    return button


if __name__ == "__main__":

    app = QApplication(sys.argv)

    window = QDialog()
    window.resize(400, 300)

    def show_message():
        QMessageBox.information(window, "Info", "You have clicked me.")

    hbox = QHBoxLayout()
    button = make_push_button("ClickMe", show_message)

    hbox.addWidget(button)
    window.setLayout(hbox)

    window.show()

    sys.exit(app.exec())


