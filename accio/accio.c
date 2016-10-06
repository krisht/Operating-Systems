/**
 * Name:    Krishna Thiyagarajan
 * Class:   ECE-357: Operating Systems
 * P.Set:   #1: System Calls
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

char* uValue = NULL;
int uSpecified;
int mValue;
int mSpecified;
int xSpecified;
int lSpecified;
char* lValue;
const char* startDir;
int devNum = -1;

void usage(void) {
    fprintf(stderr, "usage: accio [-u user] [-m mtime] [-l target] [-x]\n");
    exit(-1);
}

void findStats(const char *fileName) {
    struct stat sb;

    if (lstat(fileName, &sb) == -1) {
        perror("stat");
        exit(-1);
    }

    char type;

    switch (sb.st_mode & S_IFMT) {
        case S_IFBLK:	type = 'b';	break;
        case S_IFCHR:	type = 'c';	break;
        case S_IFDIR:	type = 'd';	break;
        case S_IFIFO:	type = 'p';	break;
        case S_IFLNK:	type = 'l';	break;
        case S_IFREG:	type = '-';	break;
        case S_IFSOCK:	type = 's';	break;
        default:		exit(-1);  //exit unknown file type
    }


    int devid = (int) sb.st_dev;
    if(devNum == -1)
        devNum = devid;
    int inNum = (int) sb.st_ino;
    long perm = (long) sb.st_mode;
    int lnkcnt = (int) sb.st_nlink;
    int uid = (int) sb.st_uid;
    char* uname = getpwuid(sb.st_uid)->pw_name;
    int gid = (int) sb.st_gid;
    char* gname  = getgrgid(sb.st_gid)->gr_name;
    int size = (int) sb.st_size;

    char s[1000];
    time_t t = sb.st_mtime;
    struct tm *p = localtime(&t);
    strftime(s, 1000, "%x %X", p);

    if(uSpecified){
        if(atoi(uValue) == 0 ){
            struct passwd *temp = getpwnam(uValue);
            int newuid = temp->pw_uid;
            if(newuid != uid)
                return;
        }
        else
            if(uid != atoi(uValue))
                return;
    }

    if(mSpecified){
        time_t currTime;
        time(&currTime);
        double diff_t = difftime(currTime, t);
        if(mValue > 0 && diff_t  > abs(mValue))
            return;
        if(mValue < 0 && diff_t < abs(mValue))
            return;
    }

    if(xSpecified && devid != devNum){
        fprintf(stderr, "note: not crossing mount point at %s", fileName);
        return;
    }

    if(lSpecified && type !='l')
    	return; 


    if(type == 'l'){
        char* linksTo = (char*)malloc(sb.st_size + 1);
        if(linksTo == NULL){
            fprintf(stderr, "Insufficient memory!\n");
            exit(EXIT_FAILURE);
        }
        int r;
        if((r = readlink(fileName, linksTo, sb.st_size + 1)) == -1){
            perror("lstat");
            exit(EXIT_FAILURE);
        }
        if (r > sb.st_size) {
            fprintf(stderr, "symlink increased in size "
                    "between lstat() and readlink()\n");
            exit(EXIT_FAILURE);
        }
        linksTo[r] = '\0';

        if(lSpecified){
            struct stat temp;
            struct stat temp2;
            if (lstat(lValue, &temp) == -1 || lstat(linksTo, &temp2) == -1) {
                perror("stat");
                exit(-1);
            }

            if(temp.st_dev != temp2.st_dev || temp.st_ino != temp2.st_ino)
                return;

        }

        printf("%04x/%-10d %2c %7ld %10d %10s %10s %10d %10s %s -> %s\n", devid, inNum, type, perm, lnkcnt,
               getpwuid(sb.st_uid)->pw_name, getgrgid(sb.st_gid)->gr_name, size, s, fileName, linksTo);
        free(linksTo);
    }
    else
        printf("%04x/%-10d %2c %7ld %10d %10s %10s %10d %10s %s\n", devid, inNum, type, perm, lnkcnt,
               getpwuid(sb.st_uid)->pw_name, getgrgid(sb.st_gid)->gr_name, size, s, fileName);
}

void fileWalker(const char *dir_name) {
    DIR * d;
    d = opendir (dir_name);
    if (!d) {
        fprintf (stderr, "Cannot open directory '%s': %s\n",
                 dir_name, strerror (errno));
        exit (EXIT_FAILURE);
    }
    while (1) {
        struct dirent * entry;
        const char * d_name;
        entry = readdir (d);
        if (! entry)
            break;
        d_name = entry->d_name;
        int path_length;
        char path[PATH_MAX];
        path_length = snprintf (path, PATH_MAX, "%s/%s", dir_name, d_name);
        if (path_length >= PATH_MAX) {
            fprintf (stderr, "Path length has got too long.\n");
            exit (EXIT_FAILURE);
        }

        findStats(path);

        if (entry->d_type == DT_DIR && entry->d_type != DT_SOCK)
        if (strcmp (d_name, "..") != 0 && strcmp (d_name, ".") != 0)
            fileWalker (path);
    }
    if (closedir (d)) {
        fprintf (stderr, "Could not close '%s': %s\n", dir_name, strerror (errno));
        exit (EXIT_FAILURE);
    }
}


int main(int argc, char **argv) {
    char ch;
    while ((ch = getopt(argc, argv, ":xu:m:l:")) != -1)
        switch (ch) {
            case 'u': 	uValue = optarg;
                        uSpecified = 1;
                        break;
            case 'm':	mValue = atoi(optarg);
                        mSpecified = 1;
                        break;
            case 'x':	xSpecified = 1;
                        break;
            case 'l': 	lValue = optarg;
                        lSpecified = 1;
                        break;
            case ':': 	printf("Input expeced at -%c!\n", optopt); break;
            case '?':	exit(-1);
            default :	usage();
        }

    startDir = optind == argc ? "." : argv[optind];
    fileWalker(startDir);
    exit(0);
}