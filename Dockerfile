FROM ubuntu
RUN apt update 
RUN apt install g++ -qy
COPY copy_directory.cpp .
RUN g++ copy_directory.cpp -o copy_directory -pthread
CMD ./copy_directory /etc /home && ls /home
