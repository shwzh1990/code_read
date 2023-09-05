.PHONY:getcode login
login:
	./log_in.sh
mount:
	sshfs pi@192.168.20.19:/home/pi/Desktop/code_learn/ ~/Desktop/remote_project
unmount:
	fusermount -u ~/Desktop/remote_project
