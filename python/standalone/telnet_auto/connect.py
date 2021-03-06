#!/usr/bin/env python3

import sys
import telnet


#파일을 한줄씩 읽어서 메모리에 저장
class Tel():

	def fileget(self,file_name, list_line):
		f = open(file_name, 'r') #파일 열고
		while 1:
			line = f.readline() #파일을 한 라인씩 읽고

			if not line: # 라인이 더이상 없으면 멈추고
				break

			list_line.append(line) #라인을 리스트 차례대로 저장



	def telnetResult(self,list_line):

		login_prompt = bytes("login:",'utf-8')
		passwd_prompt = bytes("Password:",'utf-8') #password prompt 패턴이랑 다르면 알아서 바꾸면됨.
		command_prompt = bytes("$",'utf-8') #커맨드 프롬프트(보통 스위치에 접속하면 > 로 프롬프트가 뜨던데 테스트 환경에서는 $ 로 뜸 알아서 바꾸면 됨.

		timeout = 10 #타임아웃이 너무 길면 타임아웃도 알아서 줄이면 됨. 너무 짧으면 안될수도 있음 줄여도 적당히
		port = 23 #포트도 알아서..

		f = open("result.log", 'w') #저장할 파일 열고

		for line in list_line: #리스트 만큼 돌면서
			f.write(line +"\n") #파일 첫줄에 장비정보 쓰고
            
			if self.linecheck(line) == False:
				f.write("invalid Token \n")
				continue
				
			lineResult = line.split(",") # 파일 한라인을 공백 기준으로 짜르고
			ip = lineResult[0].strip() #라인 첫항목
			userid = bytes(lineResult[1].strip()+"\n",'utf-8') #라인 두번째항목
			passwd = bytes(lineResult[2].strip()+"\n",'utf-8') #라인 세번째항목
			command = bytes(lineResult[3].strip()+"\n",'utf-8') #라인 네번째 항목
			print(command)

			#telnet open
			tn = telnet.Telnet(ip, port, timeout) #텔넷 열고

			if tn.sock is None: #텔넷 소켓 열기 실패하면 타임아웃(ip가 다르던가 타임아웃)
				f.write("socket open failed \n")
				f.write("=============================================== \n")
				continue # 텔넷 소켓열기 실패하면 다음 장비로 넘어감.
            
			response = tn.read_until(login_prompt, timeout)# 텔넷 열기 실패하지 않았을 경우 로그인프롬프트 체크
            
			if login_prompt not in response: #받은 결과가 로그인 프롬프트가 아니면
				#print "Error: get login prompt fail"
				f.write("Error: get login prompt fail \n") # 실패에러메시지 파일에 쓰고
				f.write("=============================================== \n")
				tn.close() #텔넷 닫고
				continue #다음장비로 넘어감.
				#ys.exit(1)

			tn.write(userid) #로그인프롬프트가 떳을 경우 로그인아이디 입력

			response = tn.read_until(passwd_prompt, timeout) #패스워드 프롬프트 체크
			if passwd_prompt not in response: #패스워드 프롬프트가 안뜨면
				#print "Error: get password prompt fail"
				f.write("Error: get password prompt fail \n") #파일에 에러메시지 입력
				f.write("=              ============================================= \n")
				tn.close() #텔넷 닫고
				continue #다음장비
				#sys.exit(1)

			tn.write(passwd) #패스워드 프롬프트가 뜨면 패스워드 입력

			# 주석달기 귀찮음....어차피 패턴은 다 똑같음.. 알아서 필요하면 바꿔서 쓰길바람...
			response = tn.read_until(command_prompt, timeout)            
			#test = response.find("incorrect")
       		#response_str = response.decode()
			if response.find(b"incorrect") > 0:
				#print("invalid password")
				f.write("invalid password \n")

			if command_prompt not in response:
				#print "Error: get command prompt fail"
				f.write("Error: get command prompt fail \n")
				f.write("=============================================== \n")
				tn.close()
				continue
				#sys.exit(1)

			tn.write(command)

			#tn.close()

			response = tn.read_until(command_prompt)

			f.write(response.decode('utf-8')+"\n")
			f.write("============================================ \n")
			tn.close()

			tn = None

		f.close()


	def main(name):
		list_line = list()    
		tel = Tel()
        
		file_name = name
		tel.fileget(file_name, list_line)
		tel.telnetResult(list_line)

	def linecheck(self,line):
		cnt = line.count(',')
		if cnt == 3:
			return True
		else:
			return False

	#if __name__ == '__main__':
	#	main()
