#include "wrapper.h"

int main(void) {
    char *buf, *p;
    char content[MAXLINE];
    int m=5, n=5;

    scanf("%d&%d", &m, &n);
    
    sprintf(content, "Multiplication: ");
    sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content);
    sprintf(content, "%sThe answer is: %d * %d = %d\r\n<p>", 
	    content, m, n, m * n);
    sprintf(content, "%sThanks!\r\n", content);

    printf("Content-length: %d\r\n", strlen(content));
    printf("Content-type: text/html\r\n\r\n");
    printf("%s", content);
    fflush(stdout);
    exit(0);
}

