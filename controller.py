import keyboard
import urllib.request

def turn(forward, backward, left, right, stop):
    urllib.request.urlopen('http://192.168.4.1/data/?sensor_reading={"forward":"'+forward+'","backward":"'+backward+'","left":"'+left+'","right":"'+right+'","stop"+"'+stop+'"')

def main():
    while True:
        keyboard.on_press_key("up", lambda : turn(1,0,0,0,0))
        keyboard.on_press_key("down", lambda : turn(0,1,0,0,0))
        keyboard.on_press_key("left", lambda : turn(0,0,1,0,0))
        keyboard.on_press_key("right", lambda : turn(0,0,0,1,0))


if __name__ == "_main__":
    main()
