#include "hadoop.h"

int hadoopReadFile()
{
	hdfsFS fs = NULL;
	hdfsConnInfo_t connInfo;
	in_t in;
	out_t out;
	char* buffer = NULL;

	//메모리 초기화
	memset(&connInfo, 0x00, sizeof(hdfsConnInfo_t));
	memset(&in, 0x00, sizeof(in_t));
	memset(&out, 0x00, sizeof(out_t));
	memset(&fs,0x00, sizeof(hdfsFS));

	/* 접속정보 설정 */
	if(setConInfo(&connInfo) == ERROR)
	{
		printf("setConInfo() ERROR \n");
		return ERROR;
	}

	/* 입력정보 설정 */
	if(setIn(&in) == ERROR)
	{
		printf("setIn ERROR() \n");
		return ERROR;
	}

	/* hadoop Connect */
	if(hadoopConnection(&fs, &connInfo, &in) == ERROR)
	{
		printf("hadoopConnection() ERROR \n");
		return ERROR;
	}

	/* hadoop file 정보 읽기 */
	if(getHadoopFileInfo(&fs, &in, &out)== ERROR)
	{
		printf(" getHadoopFileInfo() ERROR \n");
		return ERROR;
	}

	/* Buffer Memory Allocate */
	buffer = malloc(sizeof(char) * out.fileSize);

	/* hadoop 파일 읽기 */
	if(hadoopfileget(&fs, &in , &out, buffer) == ERROR)
	{
		printf("hadoopfileget() ERROR \n");
		free(buffer);
		return ERROR;

	}

	/* out 데이터를 Hadoop 에서 읽은 버퍼데이터의 포인터를 할당 */
	out.data = buffer;

	/* print */
	if(printOut(&out) == ERROR)
	{
		printf("printOut() ERROR \n");
		free(buffer);
		return ERROR;
	}

	/* test Hadoop 에서 읽은 파일을 로컬에 쓰기 */
	if(__testFileCreate(&out) == ERROR)
	{
	       printf("printOut() ERROR \n");
		   free(buffer);
		   return ERROR;
	}

	/* memory Free */
	free(buffer);


	return SUCCESS;
}


int setConInfo(hdfsConnInfo_t* connInfo)
{
	strcpy(connInfo->primaryIp,PRIMARYIP);
	connInfo->primaryPort = PRIMARYPORT;

	strcpy(connInfo->backupIp,BACKUPIP);
	connInfo->backupPort = BACKUPPORT;

	return SUCCESS;
}

int setIn(in_t* in)
{
	strcpy(in->filePath,"/test.dat");
	return SUCCESS;
}


int hadoopConnection(hdfsFS* fs, hdfsConnInfo_t *connInfo, in_t* in)
{
	int exists;

	*fs = hdfsConnect(connInfo->primaryIp, connInfo->primaryPort);

	if(!*fs)
	{
		fprintf(stderr, "Oops! Failed to connect to primary node hdfs!\n");
		return ERROR;
	}

	exists = hdfsExists(*fs, in->filePath);

	if(exists > -1)
	{
		printf("primary node file exists \n");
		return SUCCESS;
	}else
	{
		printf("primary node file not  exists reconnect backup node \n");
		*fs = hdfsConnect(connInfo->backupIp, connInfo->backupPort);

		if(!*fs)
		{
			fprintf(stderr, "Oops! Failed to connect to backup node hdfs!\n");
			return ERROR;
		}

		exists = hdfsExists(*fs, in->filePath);
		if(exists > -1)
		{
			printf("backup node file exists \n");
			return SUCCESS;
		}else
		{
			printf("backup node file not exists \n");
			return ERROR;
		}
	}


	return SUCCESS;
}


int hadoopfileget(hdfsFS* fs, in_t* in, out_t* out, char* buffer)
{
    tSize bufferSize = out->fileSize;

    hdfsFile readFile = hdfsOpenFile(*fs, in->filePath, O_RDONLY, bufferSize, 0, 0);
    if (!readFile)
    {
	fprintf(stderr, "Failed to open %s for writing!\n", in->filePath);
	exit(-2);
    }


    fprintf(stderr, "hdfsAvailable: %d\n", hdfsAvailable(*fs, readFile));

    tOffset seekPos = 1;
    if(hdfsSeek(*fs, readFile, seekPos)) {
        fprintf(stderr, "Failed to seek %s for reading!\n", in->filePath);
        exit(-1);
    }

    tOffset currentPos = -1;
    if((currentPos = hdfsTell(*fs, readFile)) != seekPos) {
        fprintf(stderr,
                "Failed to get current file position correctly! Got %ld!\n",
                currentPos);
        exit(-1);
    }
    fprintf(stderr, "Current position: %ld\n", currentPos);

    printf(" buffer size : %d \n",out->fileSize);

    tSize num_read_bytes = hdfsRead(*fs, readFile, (void*)buffer,
            out->fileSize);
    fprintf(stderr, "Read following %d bytes: \n",num_read_bytes);
 
    num_read_bytes = hdfsPread(*fs, readFile, 0, (void*)buffer,
            out->fileSize);

    fprintf(stderr, "Read following %d bytes: \n",num_read_bytes);

    /* close File */
    hdfsCloseFile(*fs, readFile);

    /* hdfsDisconnect */
    hdfsDisconnect(*fs);


#if 0
   	tSize bufferSize = 15;
    hdfsFile readFile = hdfsOpenFile(*fs, in->filePath, O_RDONLY, bufferSize, 0, 0);
	if (!readFile) {
		fprintf(stderr, "Failed to open %s for writing!\n", in->filePath);
		exit(-2);
    }
	// data to be written to the file
	char* buffer = malloc(sizeof(char) * bufferSize);
	if(buffer == NULL) {
		return -2;
    }
	// read from the file
    tSize curSize = bufferSize;
	for (; curSize == bufferSize;) {
        curSize = hdfsRead(*fs, readFile, (void*)buffer, curSize);
    }
	printf("%s",buffer);
	free(buffer);
	hdfsCloseFile(*fs, readFile);
	hdfsDisconnect(*fs);
#endif

	return SUCCESS;
}


int printOut(out_t* out)
{
	printf("fileSize : %ld \n ",out->fileSize);
	//printf("Buffer Data \n");
	printf("data : %s \n", out->data);

	return SUCCESS;
}


int getHadoopFileInfo(hdfsFS* fs, in_t* in, out_t* out)
{
	 /*FileInfomation Get */
 	hdfsFileInfo *fileInfo = NULL;

 	if((fileInfo = hdfsGetPathInfo(*fs, in->filePath)) != NULL)
 	 {
            fprintf(stderr, "hdfsGetPathInfo - SUCCESS!\n");
            fprintf(stderr, "Name: %s \n", fileInfo->mName);
            fprintf(stderr, "Type: %c \n", (char)(fileInfo->mKind));
            fprintf(stderr, "Replication: %d \n", fileInfo->mReplication);
            fprintf(stderr, "BlockSize: %ld \n", fileInfo->mBlockSize);
            fprintf(stderr, "Size: %ld \n", fileInfo->mSize);
            fprintf(stderr, "LastMod: %s \n", ctime(&fileInfo->mLastMod));
            fprintf(stderr, "Owner: %s \n", fileInfo->mOwner);
            fprintf(stderr, "Group: %s \n", fileInfo->mGroup);

            fprintf(stderr, "long Modify Date : %ld \n", fileInfo->mLastMod);
            //char permissions[10];
            //permission_disp(fileInfo->mPermissions, permissions);
            //fprintf(stderr, "Permissions: %d (%s)\n", fileInfo->mPermissions, permissions);

            out->fileSize = fileInfo->mSize;
            hdfsFreeFileInfo(fileInfo, 1);
     }
	 return SUCCESS;
}


int __testFileCreate(out_t* out)
{

    FILE *fp = fopen("test.dat", "w");    // hello.txt 파일을 쓰기 모드(w)로 열기.

    fputs(out->data, fp);   // 파일에 문자열 저장

    fclose(fp);    // 파일 포인터 닫기

    return SUCCESS;
}
