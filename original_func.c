#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "original.h"
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>

int getkey(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);
	if (ch != EOF) {
		return ch;
	}
	return 0;
}

void *play_sound(void *arg) {
    while (1) {
        if(system("play phone.wav --no-show-progress 2>/dev/null") != 0) {
            perror("system");
            exit(1);
        };
    }
    return NULL;
}

int isAllTs(const unsigned char *input)
{
    int length = strlen(input);
    if (length < 1)
    {
        return 0;
    }
    for (int i = 0; i < 10; i++)
    {
        if (input[i] != 'T')
        {
            return 0;
        }
    }
    return 1;
}

int isAllAs(const unsigned char *input)
{
    int length = strlen(input);
    if (length < 1)
    {
        return 0;
    }
    for (int i = 0; i < 10; i++)
    {
        if (input[i] != 'A')
        {
            return 0;
        }
    }
    return 1;
}

int isAllFs(const unsigned char *input)
{
    int length = strlen(input);
    if (length < 1)
    {
        return 0;
    }
    for (int i = 0; i < 10; i++)
    {
        if (input[i] != 'F')
        {
            return 0;
        }
    }
    return 1;
}

int isAllKs(const unsigned char *input)
{
    int length = strlen(input);
    if (length < 1)
    {
        return 0;
    }
    for (int i = 0; i < 10; i++)
    {
        if (input[i] != 'K')
        {
            return 0;
        }
    }
    return 1;
}

int conversation(int s, int N)
{
    FILE *fp_rec = popen("rec --no-show-progress -t raw -b 16 -c 1 -e s -r 44100 - 2>/dev/null", "r");
    if (fp_rec == NULL)
    {
        perror("popen rec");
        return -1;
    }
    FILE *fp_play = popen("play --no-show-progress -t raw -b 16 -c 1 -e s -r 44100 -", "w");
    if (fp_play == NULL)
    {
        perror("popen play");
        return -1;
    }

    fd_set readfds, writefds;
    int maxfd = s > fileno(fp_rec) ? s : fileno(fp_rec);
    maxfd = maxfd > fileno(fp_play) ? maxfd : fileno(fp_play);
    maxfd = maxfd > STDIN_FILENO ? maxfd : STDIN_FILENO;
    maxfd = maxfd > STDOUT_FILENO ? maxfd : STDOUT_FILENO;
    int speak_n;
    int listen_n;

    char mode = 0;
    char buffer[16384];

    char *all_Ts = (char *)malloc((N) * sizeof(char));
    memset(all_Ts, 'T', 100);
    char *all_As = (char *)malloc((N) * sizeof(char));
    memset(all_As, 'A', 100);
    char *all_Fs = (char *)malloc((N) * sizeof(char));
    memset(all_Fs, 'F', 100);
    char *all_Ks = (char *)malloc((N) * sizeof(char));
    memset(all_Ks, 'K', 1000);
    printf("mode is audio\n");
    while (1)
    {
        unsigned char speak[N];
        unsigned char listen[N];
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(s, &readfds);
        FD_ZERO(&writefds);
        FD_SET(s, &writefds);


        // char key = getkey();

        // speak_n = fread(speak, sizeof(unsigned char), N, fp_rec);
        // if (speak_n == -1) {
        //     perror("fread");
        //     exit(1);
        // }
        // if (speak_n == 0) break;

        // if (key) {
        //     printf("You entered %c\n", key); //debug
        //     if (key == 'm') {
        //         if (mute_mode) {
        //             printf("Already mute mode\n");
        //         }
        //         else {
        //             mute_mode = 1;
        //             printf("Moved to mute mode\n");
        //         }
        //     }
        //     if (key == 's') {
        //         if (mute_mode) {
        //             mute_mode = 0;
        //             printf("Moved to speaker mode\n");
        //         }
        //         else {
        //             printf("Already speaker mode\n");
        //         }
        //     }
        //     if (key == 'q') break;
        // }

        // if (mute_mode) memset(speak, 0, speak_n);

        if (mode == 0)
        {
            FD_SET(fileno(fp_rec), &readfds);
            FD_SET(fileno(fp_play), &writefds);
            int ret = select(maxfd + 1, &readfds, &writefds, NULL, NULL);
            if (ret == -1)
            {
                perror("select");
                pclose(fp_rec);
                pclose(fp_play);
                return 1;
            }

            if (FD_ISSET(s, &readfds))
            {
                listen_n = recv(s, listen, N, 0);
                if (listen_n == -1)
                {
                    perror("recv");
                    exit(1);
                }
                if (listen_n == 0)
                    break;
                if (isAllTs(listen))
                {
                    mode = 1;
                    printf("mode is changed to text\n");
                }
                else if (isAllFs(listen))
                {
                    mode = 3;
                    printf("mode is changed to receive file\n");
                }
                else
                {
                    if (fwrite(listen, sizeof(unsigned char), listen_n, fp_play) != listen_n)
                    {
                        perror("fwrite");
                        exit(1);
                    }
                }
            }

            if (FD_ISSET(s, &writefds))
            {
                speak_n = fread(speak, sizeof(unsigned char), N, fp_rec);
                if (speak_n == -1)
                {
                    perror("fread");
                    exit(1);
                }
                if (speak_n == 0)
                    break;

                if (send(s, speak, speak_n, 0) != speak_n)
                {
                    perror("send");
                    exit(1);
                }
            }
        }
        else if (mode == 1)
        {
            fread(buffer, 1, sizeof(buffer), fp_rec);
            fread(buffer, 1, sizeof(buffer), fp_play);
            memset(speak, 0, N);
            memset(listen, 0, N);

            int ret = select(maxfd + 1, &readfds, &writefds, NULL, NULL);
            if (ret == -1)
            {
                perror("select");
                pclose(fp_rec);
                pclose(fp_play);
                return 1;
            }
            if (FD_ISSET(s, &readfds))
            {
                unsigned char read_text[N];
                int read_n = recv(s, read_text, N, 0);
                if (read_n == -1)
                {
                    perror("recv");
                    exit(1);
                }
                if (read_n == 0)
                    break;
                if (isAllAs(read_text))
                {
                    mode = 0;
                    printf("mode is changed to audio\n");
                }
                else if (isAllFs(read_text))
                {
                    mode = 3;
                    printf("mode is changed to receive file\n");
                }
                else
                {
                    printf("received message: ");
                    fwrite(read_text, sizeof(unsigned char), read_n, stdout);
                }
            }
        }
        else if (mode == 2)
        {
            fread(buffer, 1, sizeof(buffer), fp_rec);
            fread(buffer, 1, sizeof(buffer), fp_play);
            memset(speak, 0, N);
            memset(listen, 0, N);

            int ret = select(maxfd + 1, &readfds, &writefds, NULL, NULL);
            if (ret == -1)
            {
                perror("select");
                pclose(fp_rec);
                pclose(fp_play);
                return 1;
            }
            if (FD_ISSET(s, &readfds))
            {
                unsigned char read_text[N];
                int read_n = recv(s, read_text, N, 0);
                if (read_n == -1)
                {
                    perror("recv");
                    exit(1);
                }
                if (read_n == 0)
                    break;
                if (isAllAs(read_text))
                {
                    mode = 0;
                    printf("mode is changed to audio\n");
                }
                else if (isAllTs(read_text))
                {
                    mode = 1;
                    printf("mode is changed to text\n");
                }
                else if (fwrite(read_text, sizeof(unsigned char), read_n, stdout) != read_n)
                {
                    perror("fwrite");
                    exit(1);
                }
            }
        }
        else if (mode == 3)
        {
            int ret = select(maxfd + 1, &readfds, &writefds, NULL, NULL);
            if (ret == -1)
            {
                perror("select");
                pclose(fp_rec);
                pclose(fp_play);
                return 1;
            }
            fread(buffer, 1, sizeof(buffer), fp_rec);
            fread(buffer, 1, sizeof(buffer), fp_play);
            memset(speak, 0, N);
            memset(listen, 0, N);
            printf("preparing to receive file\n");

            time_t t = time(NULL);
            struct tm *tm_info = localtime(&t);
            char filename[256];
            strftime(filename, sizeof(filename), "sended_file_%Y-%m-%d_%H-%M-%S.txt", tm_info);
            FILE *sended_fp = fopen(filename, "a");
            if (sended_fp == NULL)
            {
                perror("Error opening file");
                return 1;
            }
            int read_filename_n;
            while (1)
            {
                unsigned char read_text[N];
                int read_n = recv(s, read_text, N, 0);
                if (read_n == -1)
                {
                    perror("recv");
                    exit(1);
                }
                if (read_n == 0)
                    break;
                if (isAllAs(read_text))
                {
                    mode = 0;
                    printf("file sending is canceled\n");
                    printf("mode is changed to audio\n");
                    break;
                }
                else if (isAllTs(read_text))
                {
                    mode = 1;
                    printf("file sending is canceled\n");
                    printf("mode is changed to text\n");
                    break;
                }
                else if (isAllKs(read_text))
                {
                    mode = 0;
                    printf("Received file!\n");
                    printf("mode is changed to audio\n");
                    break;
                }
                else if (fwrite(read_text, sizeof(unsigned char), read_n, sended_fp) != read_n)
                {
                    perror("fwrite");
                    exit(1);
                }
            }
            fclose(sended_fp);
        }

        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            char input[N];
            memset(input, 0, N);
            int n;
            n = read(0, input, N);
            if (n == -1)
            {
                perror("read");
                exit(1);
            }
            if (n == 0)
            {
                break;
            }

            char input_tmp[N];
            memset(input_tmp, 0, N);
            memcpy(input_tmp, input, N);
            input_tmp[strcspn(input_tmp, "\n")] = 0; // 改行文字を取り除く
            if (mode == 2)
            {
                FILE *fp = fopen(input_tmp, "r");
                if (fp == NULL)
                {
                    perror("fopen");
                }
                else
                {
                    int read_filename_n;
                    while ((read_filename_n = fread(buffer, 1, N, fp)) > 0)
                    {
                        if (send(s, buffer, read_filename_n, 0) == -1)
                        {
                            perror("Error send data");
                            fclose(fp);
                            exit(1);
                        }
                    }
                    sleep(1);
                    printf("Send: %s!\n", input_tmp);
                    send(s, all_Ks, N, 0);
                    mode = 0;
                    printf("mode is changed to audio\n");
                }
            }
            if (strcmp(input_tmp, "1") == 0 || strcmp(input_tmp, "text") == 0)
            {
                if (mode != 1)
                {
                    mode = 1;
                    send(s, all_Ts, N, 0);
                    fread(buffer, 1, sizeof(buffer), fp_rec);
                    fread(buffer, 1, sizeof(buffer), fp_play);

                    fd_set tmp_readfds;
                    struct timeval timeout;
                    int retval;
                    while (1)
                    {
                        FD_ZERO(&tmp_readfds);
                        FD_SET(s, &tmp_readfds);
                        timeout.tv_sec = 1;
                        timeout.tv_usec = 0;
                        retval = select(s + 1, &tmp_readfds, NULL, NULL, &timeout);
                        if (retval == -1)
                        {
                            perror("select()");
                        }
                        else if (retval)
                        {
                            recv(s, buffer, sizeof(buffer), 0);
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                printf("mode is text\n");
            }
            else if (strcmp(input_tmp, "0") == 0 || strcmp(input_tmp, "audio") == 0)
            {
                if (mode != 0)
                {
                    mode = 0;
                    send(s, all_As, N, 0);
                    fread(buffer, 1, sizeof(buffer), fp_rec);
                    fread(buffer, 1, sizeof(buffer), fp_play);
                }
                printf("mode is audio\n");
            }
            else if (strcmp(input_tmp, "2") == 0 || strcmp(input_tmp, "file") == 0)
            {
                if (mode != 2)
                {
                    mode = 2;
                    send(s, all_Fs, N, 0);
                    fread(buffer, 1, sizeof(buffer), fp_rec);
                    fread(buffer, 1, sizeof(buffer), fp_play);

                    fd_set tmp_readfds;
                    struct timeval timeout;
                    int retval;
                    while (1)
                    {
                        FD_ZERO(&tmp_readfds);
                        FD_SET(s, &tmp_readfds);
                        timeout.tv_sec = 1;
                        timeout.tv_usec = 0;
                        retval = select(s + 1, &tmp_readfds, NULL, NULL, &timeout);
                        if (retval == -1)
                        {
                            perror("select()");
                        }
                        else if (retval)
                        {
                            recv(s, buffer, sizeof(buffer), 0);
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                printf("mode is sending file\n");
            }
            else if (mode == 1)
            {
                send(s, input, N, 0);
                printf("send complete!: %s", input);
            }
            else
            {
                printf("invalid command: %s", input);
            }
        }
    }

    fclose(fp_rec);
    fclose(fp_play);
    close(s);
    free(all_As);
    free(all_Ts);
    free(all_Fs);
    free(all_Ks);
    return 0;
}

int make_socket_for_server(int port_number)
{
    // bind
    int ss = socket(PF_INET, SOCK_STREAM, 0);
    if (ss == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_number);

    int bind_error = bind(ss, (struct sockaddr *)&addr, sizeof(addr));
    if (bind_error == -1)
    {
        perror("bind");
        close(ss);
        exit(1);
    }

    printf("Connecting......\n");

    // listen
    if (listen(ss, 100) == -1)
    {
        perror("listen");
        close(ss);
        exit(1);
    }

    int flags = fcntl(ss, F_GETFL, 0);
    fcntl(ss, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in client_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    int s;

    pthread_t tid;
    if (pthread_create(&tid, NULL, play_sound, NULL) != 0) {
        perror("pthread_create");
        exit(1);
    }

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(ss, &readfds);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000; // 0.1 sec

        int activity = select(ss + 1, &readfds, NULL, NULL, &tv);

        if (activity < 0) {
            perror("select");
            exit(1);
        }

        if (FD_ISSET(ss, &readfds)) {
            s = accept(ss, (struct sockaddr *)&client_addr, &len);
            if (s == -1) {
                perror("accept");
            } else {
                printf("Connected!\n");
                break;
            }
        }
    }
    pthread_cancel(tid);
    pthread_join(tid, NULL);
    close(ss);
    return s;
}

int make_socket_for_client(int port_number, char *ip_number)
{
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1)
    {
        perror("socket");
        close(s);
        exit(EXIT_FAILURE);
    }

    // 次は connect
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    int aton = inet_pton(AF_INET, ip_number, &addr.sin_addr);
    if (aton == 0)
    {
        perror("inet_pton");
        close(s);
        exit(1);
    }

    addr.sin_port = htons(port_number);
    int ret = connect(s, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("client: connect failed");
        close(s);
        exit(1);
    }
    printf("Connected!\n");
    return s;
}
