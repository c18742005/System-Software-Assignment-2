#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h> 

int main(int argc, char* argv[]) {
    // Store command
    char command[500];

    // Set the user ID
    int myUID = 1001;
    gid_t supp_groups[] = {};

    int ngroups;
    gid_t *groups;
    struct passwd *pw;
    struct group *gr;

    ngroups = 10;
    groups = malloc(ngroups * sizeof(gid_t)); // Max limit of groups

    if(getgrouplist("mmcarthy", 1001, groups, &ngroups) == -1) {
        // Deal with error
    }

    // Get all groups associated with user
    for (int j = 0; j < ngroups; j++) {
        supp_groups[j] = groups[j];
        printf(" - %d", supp_groups[j]);
    }

    puts("\n");

    uid_t uid = getuid();
    uid_t gid = getgid();
    uid_t euid = geteuid();
    uid_t egid = getegid();

    printf("User ID: %d\n", uid);
    printf("Group ID: %d\n", gid);
    printf("Effective User ID: %d\n", euid);
    printf("Effective Group ID: %d\n", egid);
    printf("UID: %d\n", myUID);

    // Change from root to mmcarthy
    setgroups(10, supp_groups);
    setreuid(myUID, uid);
    setregid(myUID, gid);
    seteuid(myUID);
    setegid(myUID);

    printf("User ID: %d\n", uid);
    printf("Group ID: %d\n", gid);
    printf("Effective User ID: %d\n", euid);
    printf("Effective Group ID: %d\n", egid);
    printf("UID: %d\n", myUID);

    // try to perform a copy as mmcarthy
    strcpy(command, "cp /tmp/myFile.txt /var/www/html/Intranet/myFile.txt");
    execlp("cp", "cp", "/tmp/myFile.txt", "/var/www/html/Intanet/Sales/myFile.txt", NULL);

    if(chown("/var/www/html/Intanet/Sales/myFile.txt", myUID, myUID) == -1) {
        puts("Failed chown\n");
    }

    // Swap back to root
    myUID = 0;
    setreuid(myUID, uid);
    setregid(myUID, gid);
    seteuid(myUID);
    setegid(myUID);

    return 0;
}