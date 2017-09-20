#include <libssh/libssh.h>
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <string.h>

#define BUF_SIZE 16384

bool receiveFile(ssh_session session, std::string targetFile, std::string receivingFile){

  ssh_scp scp;
  int rc, copied=0, i=0;
  size_t bufsize;
  char *buffer;
  char buffer_read[BUF_SIZE];

  scp = ssh_scp_new(session, SSH_SCP_READ, targetFile.c_str());
  if (scp == NULL){
    std::cout << "Scp session allocation error: " << ssh_get_error(session) << std::endl;
    return false;
  }

  if (ssh_scp_init(scp) != SSH_OK){
    std::cout << "Scp session initialization error: " << ssh_get_error(session) << std::endl;
    ssh_scp_free(scp);
    return false;
  }

  if (ssh_scp_pull_request(scp) != SSH_SCP_REQUEST_NEWFILE){
    std::cout << "File information error: " << ssh_get_error(session) << std::endl;
    return false;
  }

  bufsize = ssh_scp_request_get_size(scp);
  std::cout << "Received bufsize = " << bufsize << std::endl;
  buffer = new char[bufsize];
  if (buffer == NULL){
    std::cout << "Memory allocation error" << std::endl;
    return false;
  }

  if (ssh_scp_accept_request(scp) == SSH_ERROR){
    std::cout << "Accept request error" <<std::endl;
    delete[] buffer;
    return false;
  }

  std::cout << "Copying file to buffer ..." << std::endl;

  do{
    rc = ssh_scp_read(scp, buffer_read, BUF_SIZE);
    if (rc == SSH_ERROR){
      std::cout << "rc = " << rc <<  "Data  retrieval error: " << ssh_get_error(session) << std::endl;
      delete[] buffer;
      return false;
    }

    memcpy(&buffer[copied], buffer_read, rc);
    copied += rc;
    std::cout << "Copied " << copied << std::endl;

  } while(copied < bufsize);

  std::cout << ".. Done" << std::endl;
  std::ofstream outputFile(receivingFile.c_str(), std::ofstream::binary);
  if (outputFile == NULL){
    std::cout << "File error" << std::endl;
    delete[] buffer;
    return false;
  }
  std::cout << "Copying buffer to file ..." << std::endl;
  outputFile.write(buffer, bufsize);
  std::cout << ".. Done" << std::endl;
  delete[] buffer;

  std::cout << "Freed buffer" << std::endl;
  outputFile.close();
  std::cout << "Closed file" << std::endl;
  ssh_scp_close(scp);
  std::cout << "Closed scp" << std::endl;
  ssh_scp_free(scp);
  return true;
}


int main(int argc, char **argv){
  ssh_session my_ssh_session;
  int rc;
  int port = 22;

  if(argc < 3){
    std::cout << "Usage : ./output distantFile receivingFile" << std::endl;
    return 0;
  }

  my_ssh_session = ssh_new();

  if (my_ssh_session == NULL){
    std::cout << "ssh session poop" << std::endl;
    exit(-1);
  }

  ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, "192.168.0.1");
  ssh_options_set(my_ssh_session, SSH_OPTIONS_PORT, &port);
  ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, "username");

  std::cout << "Options set" << std::endl;

  rc = ssh_connect(my_ssh_session);
  if( rc != SSH_OK ){
    std::cout << "Erreur connexion" << std::endl;
    ssh_free(my_ssh_session);
    exit(-1);
  }

  rc = ssh_userauth_password(my_ssh_session, NULL, "password");
  if( rc != SSH_AUTH_SUCCESS ){
    std::cout << "Erreur authentification" << std::endl;
    ssh_free(my_ssh_session);
    exit(-1);
  }
  std::cout << "Authenticated" << std::endl;

  if (receiveFile(my_ssh_session, std::string(argv[1]), std::string(argv[2])))
    std::cout << "Received well" << std::endl;
  else
    std::cout << "Not received well" << std::endl;

  ssh_free(my_ssh_session);
  return 0;
}
