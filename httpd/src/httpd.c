/* ARIB Yassine 3525057 Un serveur HTTP
 * 4I400 PR (Programmation Répartie)
 * Université Pierre-et-Marie-Curie
 */

#define _XOPEN_SOURCE 700
#define _REENTRANT
#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "log.h"
#include "mime.h"
#include "util.h"

/**
 * Teste si le char passé en paramètre est un espace.
 *
 * @param {char} x - Character.
 *
 * @return Boolean.
 */
#define ISspace(x) isspace((int)(x))

// Ligne "Server" pour les headers
#define SERVER_STRING "Server: pr_httpd/0.1\r\n"

pthread_mutex_t httpd_mtx = PTHREAD_MUTEX_INITIALIZER;

// Déclaration des fonctions 
void *accept_request(void *);
void bad_request(int);
void cat(int, FILE *);
void cannot_execute(int);
void error_die(const char *);
void execute_cgi(int, const char *, const char *, const char *, log_struct *);
int get_line(int, char *, int);
void headers(int, const char *);
void not_found(int);
void serve_file(int, const char *, log_struct *);
int startup(unsigned short *);
void unimplemented(int);

/**********************************************************************/
/* Fonction lancée par un thread à chaque fois qu’un client
 * se présente et traite la requête du client 
 * Parameters: une structure servant à la journalisation 
 * et contenant le socket client */
/**********************************************************************/
void *accept_request(void *parameters) {

	log_struct logs = *(log_struct *) parameters;
	logs.file_size = 0;

	// client socket file descriptor
	int client = logs.client_sock;

	// Input buffer
	char buf[1024];

	// Number of bytes read
	int numchars;

	// HTTP method name buffer
	char method[255];

	// URL buffer
	char url[255];

	// Path buffer
	char path[512];

	// Buffer indexes
	size_t i, j;

	// File status structure
	struct stat st;

	// boolean indiquant si la requête concerne un fichier exécutable
	int cgi = 0;

	// Query string pointer
	char *query_string = NULL;

	// Lis la première ligne de la requête HTTP
	numchars = get_line(client, buf, sizeof(buf));

	// Journalisation de la première ligne
	strcpy(logs.first_line, buf);
	// suppression de caractère de fin de ligne "\n"
	logs.first_line[strlen(logs.first_line) - 1] = 0;

	// Buffer indexes
	i = 0;
	j = 0;

	// On récupère le premier mot de la requête
	while (!ISspace(buf[j]) && (i < sizeof(method) - 1)) {
		// Copie du mot dans le buffer "method"
		method[i] = buf[j];

		// Incrémente les indexes
		i++;
		j++;
	}

	// ajout du caractère de fin de chaine
	method[i] = '\0';

	// Si la method n’est ni "POST" ni "GET"
	if (strcasecmp(method, "GET") && strcasecmp(method, "POST")) {
		// Send 501 response
		unimplemented(client);
		logs.http_code = 501;
		// persistance de la structure de journalisation dans  le fichier
		pthread_mutex_lock(&httpd_mtx);
		logging(logs);
		pthread_mutex_unlock(&httpd_mtx);
		// Return
		return NULL;
	}

	// Si le nom de la method est "GET",
	// Ou le nom de la method est "POST".

	// Si le nom de la method est "POST",
	if (strcasecmp(method, "POST") == 0)
		// Mode CGI à passe à vrai
		cgi = 1;

	i = 0;

	// On avance jusqu'au prochain mot autrement dit on ignore les espaces
	while (ISspace(buf[j]) && (j < sizeof(buf)))
		j++;

	// On récupère le deuxième mot de la requête "l' URL"
	while (!ISspace(buf[j]) && (i < sizeof(url) - 1) && (j < sizeof(buf))) {
		url[i] = buf[j];
		// Incrémente les indexes
		i++;
		j++;
	}

	// ajout du caractère de fin de chaine
	url[i] = '\0';

	// Si le nom de la method est "GET",
	if (strcasecmp(method, "GET") == 0) {
		query_string = url;
		// On récupère la première partie de la requête "l' URL"
		while ((*query_string != '?') && (*query_string != '\0'))
			// Increment the query sting pointer to ignore the byte
			query_string++;

		// Si on trouve le caractère “?” dans l’url ca veut dire
		// que la requête contient des paramètre
		if (*query_string == '?') {
			// On asse le mode CGI à true
			cgi = 1;

			// ajout du caractère de fin de chaine
			*query_string = '\0';
			query_string++;
		}
	}

	// On ajoute le chemin du dossier htdocs comme préfixe à l’URL
	sprintf(path, "htdocs%s", url);
	// Si le chemin se termine par un “/” ca veut dire
	// que la ressource demandée et un répertoire
	if (path[strlen(path) - 1] == '/')
		// On ajoute “index.html” à la fin du chemin
		strcat(path, "index.html");

	// Si le chemin n’existe pas on renvoie un page 404
	if (stat(path, &st) == -1) {
		// On consomme le reste de la requête
		while ((numchars > 0) && strcmp("\n", buf))
			numchars = get_line(client, buf, sizeof(buf));

		// On renvoie la page 404
		not_found(client);
		// Journalisation du code 404
		logs.http_code = 404;
	}

	// Si le chemin existe
	else {
		// Si la ressource est lisible
		if (is_readable(path)){
			// Si le chemin est un répertoire
			if (is_directory(path))
				// On ajoute “index.html” à la fin du chemin
				strcat(path, "/index.html");

			// Si la ressource est un exécutable
			if (is_executable(path))
				// On asse le mode CGI à true
				cgi = 1;

			if (!cgi)
				// Servir la ressource
				serve_file(client, path, &logs);
			else
				// Exécuter la ressource
				execute_cgi(client, path, method, query_string, &logs);
		} else { //la ressource n'est pas lisible
			// On renvoie la page 403
			forbidden(client);
			// Journalisation du code 403
			logs.http_code = 403;
		}
	}

	// Fermer la socket
	close(client);

	// persistance de la structure de journalisation dans  le fichier 
	pthread_mutex_lock(&httpd_mtx);
	logging(logs);
	pthread_mutex_unlock(&httpd_mtx);
	return NULL;
}

void serve_file(int client, const char *filename, log_struct *logs) {
	FILE *resource = NULL;
	int numchars = 1;
	char buf[1024];
	buf[0] = 'A';

	while ((numchars > 0) && strcmp("\n", buf))
		numchars = get_line(client, buf, sizeof(buf));

	resource = fopen(filename, "r");

	if (resource == NULL) {
		not_found(client);
		logs->http_code = 404;
	} else {
		headers(client, filename);
		cat(client, resource);
		logs->http_code = 200;
		logs->file_size = file_size(filename);
	}

	fclose(resource);
}

/**********************************************************************/
/* Erreur 400*/
/**********************************************************************/
void bad_request(int client) {
	char buf[1024];
	sprintf(buf, "HTTP/1.1 400 BAD REQUEST\r\n");
	send(client, buf, sizeof(buf), 0);

	sprintf(buf, "Content-type: text/html\r\n");
	send(client, buf, sizeof(buf), 0);

	sprintf(buf, "\r\n");
	send(client, buf, sizeof(buf), 0);

	sprintf(buf, "<P>Your browser sent a bad request, ");
	send(client, buf, sizeof(buf), 0);

	sprintf(buf, "such as a POST without a Content-Length.\r\n");
	send(client, buf, sizeof(buf), 0);
}

void cat(int client, FILE *resource) {
	char buf[1024];
	fgets(buf, sizeof(buf), resource);
	while (!feof(resource)) {
		send(client, buf, strlen(buf), 0);
		fgets(buf, sizeof(buf), resource);
	}
}

void cannot_execute(int client) {
	char buf[1024];
	sprintf(buf, "HTTP/1.1 500 Internal Server Error\r\n");
	send(client, buf, strlen(buf), 0);

	sprintf(buf, "Content-type: text/html\r\n");
	send(client, buf, strlen(buf), 0);

	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);

	sprintf(buf, "<P>Error prohibited CGI execution.\r\n");
	send(client, buf, strlen(buf), 0);
}

void error_die(const char *sc) {
	// Print message
	perror(sc);
	// Exit with error code
	exit(1);
}

void execute_cgi(int client, const char *path, const char *method,
		const char *query_string, log_struct *logs) {
	// Buffer
	char buf[1024];

	// Pipe utilisé par le processus fils pour envoyer des données à son père
	int cgi_output[2];

	// Pipe utilisé par le processus père pour envoyer des données à son fils
	int cgi_input[2];

	// PID du fils
	pid_t pid;

	// Status "waitpid"
	int status;

	// index boucle
	int i;

	// caractère lu
	char c;

	// la taille de la réponse
	int number_of_bytes_sent = 0;

	// nombre de caractère lu
	int numchars = 1;

	// header "Content-Length"
	int content_length = -1;

	buf[0] = 'A';
	buf[1] = '\0';

	if (strcasecmp(method, "GET") == 0)
		while ((numchars > 0) && strcmp("\n", buf))
			numchars = get_line(client, buf, sizeof(buf));

	else /* POST */
	{
		numchars = get_line(client, buf, sizeof(buf));

		while ((numchars > 0) && strcmp("\n", buf)) {
			buf[15] = '\0';
			if (strcasecmp(buf, "Content-Length:") == 0)
				content_length = atoi(&(buf[16]));
			numchars = get_line(client, buf, sizeof(buf));
		}
		if (content_length == -1) {
			bad_request(client);
			return;
		}
	}

	sprintf(buf, "HTTP/1.1 200 OK\r\n");
	send(client, buf, strlen(buf), 0);

	if (pipe(cgi_output) < 0) {
		// erreur 500
		cannot_execute(client);
		logs->http_code = 500;
		return;
	}

	if (pipe(cgi_input) < 0) {
		// erreur 500
		cannot_execute(client);
		logs->http_code = 500;
		return;
	}

	// Si la création des pipes s’est bien passé on crée un processus fils
	if ((pid = fork()) < 0) {
		// erreur 500
		cannot_execute(client);
		logs->http_code = 500;
		return;
	}

	if (pid == 0) /* fils : CGI */
	{
		char meth_env[255];
		char query_env[255];
		char length_env[255];

		dup2(cgi_output[1], STDOUT_FILENO);
		dup2(cgi_input[0], STDIN_FILENO);

		close(cgi_output[0]);
		close(cgi_input[1]);

		sprintf(meth_env, "REQUEST_METHOD=%s", method);
		putenv(meth_env);

		if (strcasecmp(method, "GET") == 0) {
			sprintf(query_env, "QUERY_STRING=%s", query_string);
			putenv(query_env);
		}

		else { // POST
			sprintf(length_env, "CONTENT_LENGTH=%d", content_length);
			putenv(length_env);
		}

		execl(path, path, NULL);

		// si execl marche l'instruction ne sera pas executer
		exit(500);

	} else { // PERE

		close(cgi_output[1]);
		close(cgi_input[0]);

		if (strcasecmp(method, "POST") == 0)
			for (i = 0; i < content_length; i++) {
				recv(client, &c, 1, 0);
				write(cgi_input[1], &c, 1);
			}
		// Remarque: on n’attend pas la fin de l'exécution du fils,
		// on envoi résultat au client au fur et à mesure 
		while (read(cgi_output[0], &c, 1) > 0) {
			send(client, &c, 1, 0);
			number_of_bytes_sent++;
		}

		close(cgi_output[0]);
		close(cgi_input[1]);

		// on attent la fin du fils avec waitpid
		if (waitpid(pid, &status, 0) > 0) {
			if (WIFEXITED(status) && !WEXITSTATUS(status)) {
				// le programme a terminé son exécution sans erreur
				logs->http_code = 200;
				logs->file_size = number_of_bytes_sent;
			} else if (WIFEXITED(status) && WEXITSTATUS(status)) {
				if (WEXITSTATUS(status) == 500) {
					// erreur execl()
					cannot_execute(client);
					logs->http_code = 500;
					logs->file_size = number_of_bytes_sent;
				} else {
					// le programme a terminé son exécution
					// mais la valeur de retour est différente de zéro
					cannot_execute(client);
					logs->http_code = 500;
					logs->file_size = number_of_bytes_sent;
				}
			} else {
				// le programme a terminé son exécution correctement
				cannot_execute(client);
				logs->http_code = 500;
				logs->file_size = number_of_bytes_sent;
			}
		} else {
			// erreur waitpid()
			cannot_execute(client);
			logs->http_code = 500;
			logs->file_size = number_of_bytes_sent;
		}
	}
}

/**********************************************************************/
/* lis une ligne dans un header http se terminant par “\n” ou “\r\n”
 * (\r\n est transformée en \n)*/
/**********************************************************************/
int get_line(int sock, char *buf, int size) {
	int i = 0;
	char c = '\0';
	int n;

	while ((i < size - 1) && (c != '\n')) {
		n = recv(sock, &c, 1, 0);
		if (n > 0) {
			if (c == '\r') {
				n = recv(sock, &c, 1, MSG_PEEK);
				if ((n > 0) && (c == '\n'))
					recv(sock, &c, 1, 0);
				else
					c = '\n';
			}

			buf[i] = c;
			i++;
		} else
			c = '\n';
	}

	buf[i] = '\0';
	return (i);
}

void headers(int client, const char *filename) {
	char buf[1024];

	const char *mime = get_mime_type(filename);

	// Content-Type
	char content_type[128];
	sprintf(content_type, "Content-Type: %s\r\n", mime);
	// Content-Length
	char content_length[32];
	sprintf(content_type, "Content-Length: %d\r\n", file_size(filename));
	strcpy(buf, "HTTP/1.1 200 OK\r\n");
	send(client, buf, strlen(buf), 0);

	strcpy(buf, SERVER_STRING);
	send(client, buf, strlen(buf), 0);

	strcpy(buf, content_type);
	send(client, buf, strlen(buf), 0);

	strcpy(buf, content_length);
	send(client, buf, strlen(buf), 0);

	strcpy(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
}

/**********************************************************************/
/* Erreur 403*/
/**********************************************************************/
void forbidden(int client) {
	char buf[1024];
	sprintf(buf, "HTTP/1.1 403 FORBIDDEN\r\n");
	send(client, buf, strlen(buf), 0);

	sprintf(buf, SERVER_STRING);
	send(client, buf, strlen(buf), 0);

	sprintf(buf, "Content-Type: text/html\r\n");
	send(client, buf, strlen(buf), 0);

	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);

	sprintf(buf, "<HTML><TITLE>Forbidden</TITLE>\r\n");
	send(client, buf, strlen(buf), 0);

	sprintf(buf, "<BODY><P>Unauthorized access</P> \r\n");
	send(client, buf, strlen(buf), 0);

	sprintf(buf, "</BODY></HTML>\r\n");
	send(client, buf, strlen(buf), 0);
}
/**********************************************************************/
/* Erreur 404*/
/**********************************************************************/
void not_found(int client) {
	char buf[1024];
	sprintf(buf, "HTTP/1.1 404 NOT FOUND\r\n");
	send(client, buf, strlen(buf), 0);

	sprintf(buf, SERVER_STRING);
	send(client, buf, strlen(buf), 0);

	sprintf(buf, "Content-Type: text/html\r\n");
	send(client, buf, strlen(buf), 0);

	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);

	sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
	send(client, buf, strlen(buf), 0);

	sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
	send(client, buf, strlen(buf), 0);

	sprintf(buf, "your request because the resource specified\r\n");
	send(client, buf, strlen(buf), 0);

	sprintf(buf, "is unavailable or nonexistent.\r\n");
	send(client, buf, strlen(buf), 0);

	sprintf(buf, "</BODY></HTML>\r\n");
	send(client, buf, strlen(buf), 0);
}

/**********************************************************************/
/* initialisation (bind + listen)*/
/**********************************************************************/
int startup(unsigned short *port) {
	int httpd = 0;
	struct sockaddr_in name;
	httpd = socket(PF_INET, SOCK_STREAM, 0);

	if (httpd == -1)
		error_die("socket");

	memset(&name, 0, sizeof(name));
	name.sin_family = AF_INET;
	name.sin_port = htons(*port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(httpd, (struct sockaddr *) &name, sizeof(name)) < 0)
		error_die("bind");

	if (*port == 0) {
		socklen_t namelen = sizeof(name);
		if (getsockname(httpd, (struct sockaddr *) &name, &namelen) == -1)
			error_die("getsockname");
		*port = ntohs(name.sin_port);
	}

	if (listen(httpd, 5) < 0)
		error_die("listen");
	return (httpd);
}

/**********************************************************************/
/* Erreur 501*/
/**********************************************************************/
void unimplemented(int client) {
	char buf[1024];

	sprintf(buf, "HTTP/1.1 501 Method Not Implemented\r\n");
	send(client, buf, strlen(buf), 0);

	sprintf(buf, SERVER_STRING);
	send(client, buf, strlen(buf), 0);

	sprintf(buf, "Content-Type: text/html\r\n");
	send(client, buf, strlen(buf), 0);

	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);

	sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
	send(client, buf, strlen(buf), 0);

	sprintf(buf, "</TITLE></HEAD>\r\n");
	send(client, buf, strlen(buf), 0);

	sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
	send(client, buf, strlen(buf), 0);

	sprintf(buf, "</BODY></HTML>\r\n");
	send(client, buf, strlen(buf), 0);
}

/**********************************************************************/
/**
 * Fonction main.
 */
/**********************************************************************/

int main(void) {

	// Numéro du port d'écoute.
	unsigned short port = 8080;

	// Structure pour l’adresse du client
	struct sockaddr_in client_name;

	// Taille de la structure
	socklen_t client_name_len = sizeof(client_name);

	// Pointer de thread
	pthread_t newthread;

	// FD Server socket
	int server_sock;

	// PID Serveur
	int server_pid = getpid();

	// Crée socket serveur
	server_sock = startup(&port);

	// Message d'info
	printf("httpd running on port %d\n", port);

	// Boucle infini
	while (1) {
		// Paramètre à passé au thread pour effectuer une Journalisation
		struct log_struct logs;

		// Accepte une requête
		logs.client_sock = accept(server_sock, (struct sockaddr *) &client_name,
				&client_name_len);

		// Journalisation ip
		strcpy(logs.ip, inet_ntoa(client_name.sin_addr));

		// Journalisation date
		time_t t1 = time(NULL);
		struct tm tm = *localtime(&t1);
		sprintf(logs.date, "%d-%d-%d %d:%d:%d", tm.tm_year + 1900,
				tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

		// Journalisation pid serveur
		logs.serveur_pid = server_pid;

		// Si on à une erreur
		if (logs.client_sock == -1)
			// On affiche l'erreur et on quitte le programme
			error_die("accept");

		// Créer un thread et traiter la requête 
		// Si on à une erreur
		if (pthread_create(&newthread, NULL, accept_request, &logs) != 0)
			// On affiche l'erreur et on quitte le programme
			perror("pthread_create");

		logs.thread_id = (int) newthread;
	}
	// Ferme le socket du serveur
	close(server_sock);

	// fin sans erreur
	return (0);
}
