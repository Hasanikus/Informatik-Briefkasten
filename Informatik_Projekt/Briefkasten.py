#!/usr/bin/env python3

import tkinter as tk
import socket as so

class Arduino(object):
    def __init__(self, window, host, port, on_received):

        self.window= window
        self.on_received= on_received

        """Socket wird freigegeben und es kann zum Arduino verbunden werden"""
        self.socket= so.socket()
        self.socket.connect((host, port))
        self.socket.setblocking(False)

        self.rd_buff= bytes()

        self._periodic_socket_check()

    def send_command(self, command):
        """Signal für Arduino"""

        self.socket.send(command.encode('utf-8') + b'\n')

    def close(self):
        """Abbruch der Verbindung"""

        self.socket.close()
        self.window.after_cancel(self.after_event)

    def _periodic_socket_check(self):
        try:
            msg= self.socket.recv(1024)

            if not msg:
                raise(IOError('Connection closed'))

            self.rd_buff+= msg

        except so.error:

            pass

        while b'\n' in self.rd_buff:
            line, self.rd_buff= self.rd_buff.split(b'\n')

            line= line.decode('utf-8').strip()
            self.on_received(line)

       
        self.after_event= self.window.after(     """tkinter führt die Funktion jede 100ms aus"""
            100, self._periodic_socket_check
        )

class OpenButton(object):

    def __init__(self, window, arduino): """Hier wird ein Button mit dem Text Open erstellt mit der Farbe grün"""
        self.button= tk.Button(          
            window,
            text='Open', fg="green",
            command= self.on_pressed
        )

        self.button.pack()

        self.arduino= arduino

        self.set_state(False)
    
    def set_state(self, state):
        if state:
            self.arduino.send_command('on') """in beiden states wird beim Knopfdruck der command 'on' zum Arduino geschickt""" 
            self.state= True                
        else:
            self.arduino.send_command('on')
            self.state= False

    def on_pressed(self):
        self.set_state(not self.state)

class CloseButton(object):
    
    def __init__(self, window, arduino):  """Hier wird ein Button mit dem Text Close erstellt mit der Farbe rot"""
        self.button= tk.Button(           
            window,
            text='Close', fg="red",
            command= self.on_pressed
        )

        self.button.pack()

        self.arduino= arduino

        self.set_state(False)
    
    def set_state(self, state):
        if state:
            self.arduino.send_command('off') """in beiden states wird beim Knopfdruck der command off zum Arduino geschickt"""
            self.state= True                 
                                             
        else:
            self.arduino.send_command('off')
            self.state= False

    def on_pressed(self):
        self.set_state(not self.state)

class BriefkastenWindow(object):
    def __init__(self):
        self.setup_window()

        host= input('Hostname: ') """Hier muss man die IP und den Port angeben"""
        port= input('Port: ')

        self.arduino= Arduino(  """die Verbindung zum Server wird erstellt"""
            self.window,
            host, int(port),
            self.on_received
        )

        self.setup_content()

    def on_received(self, line):
        self.btn_label_var.set('Anzahl der Briefeinwürfe: {}'.format(line """Variabler Text. Was nach dem : auftaucht ist abhängig vom Signal des Arduinos"""
    def setup_window(self):
        self.window= tk.Tk()                         """Setup für Fenstergröße und Titel"""
        self.window.geometry('175x80')               
        self.window.title('Briefkasten')
        self.window.protocol("WM_DELETE_WINDOW", self.on_close)

    def on_close(self):
        self.arduino.close()
        self.window.destroy()

    def setup_content(self):
        self.btn_label_var= tk.StringVar(self.window)        """Ein Text wird erstellt und die Anzahl der Briefe ist gleich 0 beim Start des des Programms solange ein Signal vom Arduino gesendet wird"""
        self.btn_label_var.set('Anzahl der Briefeinwürfe: 0')
                                                                
        self.btn_label = tk.Label(                           
            self.window,
            textvariable= self.btn_label_var
        )

        self.btn_label.pack()

        self.btn= OpenButton(self.window, self.arduino)     """Methoden für die beiden Buttons"""
        self.btn= CloseButton(self.window, self.arduino)

    def run(self):           """die mainloop wird ausgeführt um das Fenster für den Briefkasten zu starten"""
       

        self.window.mainloop()


if __name__ == '__main__':
    window= BriefkastenWindow()
    window.run()
   
   
