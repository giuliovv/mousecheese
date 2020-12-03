import urllib.request

def turn(forward, backward, left, right, stop):
    print(forward)
    urllib.request.urlopen('http://192.168.4.1/data/?sensor_reading={"forward":"'+str(forward)+'","backward":"'+str(backward)+'","left":"'+str(left)+'","right":"'+str(right)+'","stop":"'+str(stop)+'"}')

def main():
    while True:
        key = input("Command: ")
        if key == 'w':
            turn(1,0,0,0,0)
        if key == 'z':
            turn(0,1,0,0,0)
        if key == 'a':
            turn(0,0,1,0,0)
        if key == 's':
            turn(0,0,0,1,0)
        if key == ' ':
            turn(0,0,0,0,1)


if __name__ == "__main__":
    main()
