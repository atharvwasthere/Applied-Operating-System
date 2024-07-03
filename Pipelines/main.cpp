#include <iostream>
#include <unistd.h>

void pipeline(const char* process1, const char* process2 ){
    int fd[2];
    pipe(fd);
    int id = fork();
    if (id != 0) { //  parent process
        close(fd[0]); // close the read end of the pipe
        dup2(fd[1],STDOUT_FILENO); // Redirect standard output to thw write end of the pipe
        close(fd[1]); // close the write end of the page

        //Execute process1
        execlp("/usr/bin/cat" , "cat" , "main.cpp" , nullptr);
        std::cerr << "Failed to execute " << process1 <<std::endl;
    }else{
        close(fd[1]); // close the write end of the pipe
        dup2(fd[0],STDIN_FILENO); // Redirect standard input to the read end of the pipe
        close(fd[0]); // close the read end of the pipe

        //Execute process2
        execlp("/usr/bin/grep" , "grep", "hello" , nullptr);
        std::cerr << "Failed to execute " << process2 << std::endl;
    }

}

int  main() {
    pipeline("cat main.cpp", "grep hello");
    return 0;
}