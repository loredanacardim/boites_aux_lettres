/* librairie standard ... */
#include <stdlib.h>
/* pour getopt */
#include <unistd.h>
/* déclaration des types de base */
#include <sys/types.h>
/* constantes relatives aux domaines, types et protocoles */
#include <sys/socket.h>
/* constantes et structures propres au domaine UNIX */
#include <sys/un.h>
/* constantes et structures propres au domaine INTERNET */
#include <netinet/in.h>
/* structures retournées par les fonctions de gestion de la base de
données du réseau */
#include <netdb.h>
/* pour les entrées/sorties */
#include <stdio.h>
/* pour la gestion des erreurs */
#include <errno.h>

// Mode Local
struct sockaddr_in addr_local;
int lg_addr_local = sizeof(addr_local);

// Mode Distant
struct hostent *hp;
struct sockaddr_in adr_distant;

struct sockaddr_in adr_client;
int lg_adr_client = 0;

// Autenticaçao
struct authentication
{
	int etat, lg_msg_bal, nb_msg_bal;
};

// Carta
struct lettre
{
	int n_lettre, n_recep;
	char *message;
	struct lettre *lettre_suiv;
};
// BAL
struct bal
{
	int n_rec;
	struct lettre *lettre;
	struct bal *bal_suiv;
	struct lettre *courant;
};

// Lista BAL
struct l_bal
{
	struct bal *courant;
	struct bal *dernier;
	struct lettre *premier;
};

// Criar Caixa
struct bal *creer_boite(int num)
{
	struct bal *newbal;
	newbal = malloc(sizeof(struct bal));
	newbal->n_rec = num;
	newbal->lettre = NULL;
	newbal->bal_suiv = NULL;
	newbal->courant = newbal->lettre;
	return newbal;
};

// Criaço carta
struct lettre *creer_lettre(char *message, int num)
{
	struct lettre *newlettre;
	newlettre = malloc(sizeof(struct lettre));
	newlettre->n_lettre = 1;
	newlettre->message = message;
	newlettre->n_recep = num;
	newlettre->lettre_suiv = NULL;
	return newlettre;
};

// Add carta
void ajout_lettre(struct lettre *lettre, struct bal *boite)
{
	(boite->courant) = (boite->lettre);
	if (boite->lettre = NULL)
	{
		boite->lettre = lettre;
	}
	else
	{
		int num = 2;
		while (boite->courant->lettre_suiv != NULL)
		{
			boite->courant = boite->courant->lettre_suiv;
			num = num + 1;
		}
		boite->courant->lettre_suiv = lettre;
		lettre->n_lettre = num;
	}
}

// Criaçao do socket
int creat_socket(int udp)
{
	int sock = -1;
	if (udp == 0)
		sock = socket(AF_INET, SOCK_DGRAM, 0);
	else if (udp == 1)
		sock = socket(AF_INET, SOCK_STREAM, 0);
	else
		printf("Protocole inconnu");
}

// Contruçao da mensagem
void construire_message(char *message, char motif, int lg, int compteur)
{
	int i;
	sprintf(message, "%d", compteur);
	for (i = 5; i < lg; i++)
	{
		message[i] = motif;
	}
}

// Mostrar Mensagem
void afficher_message(char *message, int lg, int source, int num)
{
	int i;
	if (source == 1)
	{
		printf("Source: %d (%d)", num, lg);
		for (int i = 0; i < lg; i++)
		{
			printf("%c", message[i]);
			printf("\n");
		}
	}
}

// Mostrar Mensagem BAL
void aff_msg_bal(char *message, int lg, int source, int num, int receveur)
{
	int i;
	if (source == 1)
	{
		printf("Source: Envoi lettre %d a destination du recep. %d (%d)", num, receveur, lg);
		for (int i = 0; i < lg; i++)
		{
			printf("%c", message[i]);
		}
		printf("\n");
	}
	else if (source == 0)
	{
		printf("Source: Recuperation lettre par le recepteur %d (%d) ", receveur, lg);
		for (int i = 0; i < lg; i++)
		{
			printf("%c", message[i]);
		}
		printf("\n");
	}
}

void main(int argc, char **argv)
{
	int c, udp = 0, sock, sock_b, sock_local;
	extern char *optarg;
	extern int optind;
	int nb_message = 10; /* Nb de messages à envoyer ou à recevoir, par défaut : 10 en émission, infini en réception */
	int lg_message = 30, m_envoie, num = 0;
	char *message = (char *)malloc(lg_message * sizeof(char));
	int source = -1; /* 0=puits, 1=source */
	int port = atoi(argv[argc - 1]);
	char *dest = (argv[argc - 2]);
	int receveur = 0;
	int boite = -1;
	struct authentication id;
	int lg_recu = 1;




	while ((c = getopt(argc, argv, "spnu:l:be:r:")) != -1)
	{
		switch (c)
		{
		case 'p':
			if (source == 1)
			{
				printf("usage: cmd [-p|-s][-n ##]\n");
				exit(1);
			}
			source = 0;
			break;

		case 's':
			if (source == 0)
			{
				printf("usage: cmd [-p|-s][-n ##]\n");
				exit(1);
			}
			source = 1;
			break;

		case 'n':
			nb_message = atoi(optarg);
			break;

		case 'u':
			udp = 1;
			break;

		case 'e':
			source = 1;
			boite = 1;
			receveur = atoi(optarg);
			break;

		case 'b':
			source = 2;
			boite = 1;
			break;

		case 'r':
			source = 0;
			boite = 1;
			receveur = atoi(optarg);
			break;

		default:
			printf("usage: cmd [-p|-s][-n ##]\n");
			break;
		}
	}

	if (source == -1)
	{
		printf("usage: cmd [-p|-s][-n ##]\n");
		exit(1);
	}

	// UDP
	if (udp == 0)
	{
		sock = creat_socket(udp);

		// Emmeteur
		if (source == 1)
		{
			printf("On est dans le source. Emmeteur: lg_msg_emis=%d, port=%d, nb_envoi=%d, TP=udp -> %s \n ", lg_message, port, nb_message, dest);
			if (sock == -1)
			{
				printf("Echec de creation du socket \n");
				exit(1);
			}

			printf("SOURCE: socket \n");

			// Criaçao do endereço do socket
			memset((char *)&addr_local, 0, lg_addr_local);
			addr_local.sin_family = AF_INET;
			addr_local.sin_port = htons(port);

			// Endereço IP
			if ((hp = gethostbyname(dest)) == NULL)
			{
				printf("erreur gethostbyname \n");
				exit(1);
			}
			memcpy((char *)&(adr_distant.sin_addr.s_addr),
				   hp->h_addr,
				   hp->h_length);

			m_envoie = nb_message;

			while (m_envoie != 0)
			{
				m_envoie--;
				num++;
				construire_message(message, lg_message, source, num);
				if (sendto(sock, message, strlen(message), 0, (struct sockaddr *)&adr_distant, sizeof(adr_distant)) < 0)
				{
					printf("Erreur Sendto");
				}
				afficher_message(message, lg_message, source, num);
			}
			num = 0;
			printf("Source: Fin!! \n");
		}

		// Recepteur
		else if (source == 0)
		{
			printf("PUITS: lg_m_lu=%d, port=%d, n_receptions=%d, TP=udp \n", lg_message, port, nb_message);

			if (sock == -1)
			{
				printf("Echec de création du socket. \n");
				exit(1);
			}
			printf("PUITS: socket \n");

			// Criaçao endereço do socket
			memset((char *)&addr_local, 0, lg_addr_local);
			addr_local.sin_family = AF_INET;
			addr_local.sin_port = htons(port);
			addr_local.sin_addr.s_addr = INADDR_ANY;

			// Associar o endereço ao socket
			if (bind(sock, (struct sockaddr *)&addr_local, lg_addr_local) == -1)
			{
				printf("Echec du bind.");
				exit(1);
			}

			lg_addr_local = sizeof(addr_local);

			while (nb_message != 0)
			{
				nb_message--;
				num++;
				if (recvfrom(sock, message, lg_message, 0, (struct sockaddr *)&addr_local, &lg_addr_local) < 0)
				{
					printf("Erreur recvfrom");
					exit(1);
				}
				afficher_message(message, lg_message, source, num);
			}
			num = 0;
			printf("PUITS: fin! \n");
		}
	}

	// TCP
	else if (udp == 1)
	{
		sock = creat_socket(udp);

		// Emetteur
		if (source == 1)
		{
			printf("Source: lg_msg_emis=%d, port=%d, nb_envoi=%d, TP=TCP -> %s \n", lg_message, port, nb_message, dest);

			if (sock == -1)
			{
				printf("Echec de creation du socket \n");
				exit(1);
			}

			printf("Source: socket \n");

			// Criaçao do endereço do socket
			memset((char *)&addr_local, 0, lg_addr_local);
			addr_local.sin_family = AF_INET;
			addr_local.sin_port = htons(port);

			// Endereço IP
			if ((hp = gethostbyname(dest)) == NULL)
			{
				printf("erreur gethostbyname \n");
				exit(1);
			}
			memcpy((char *)&(adr_distant.sin_addr.s_addr),
				   hp->h_addr,
				   hp->h_length);

			if (connect(sock, (struct sockaddr *)&adr_distant, sizeof(adr_distant)) == 0)
			{
				printf("Connect \n");

				// Construir a mensagem do identificador
				id.etat = source;
				id.lg_msg_bal = lg_message;
				id.nb_msg_bal = nb_message;

				// Envia a mensagem do indentificador
				if (send(sock, &id, sizeof(&id), 0) < 0)
				{
					printf("Erreur send");
					exit(1);
				}
				m_envoie = nb_message;

				while (m_envoie != 0)
				{
					m_envoie--;
					num++;

					construire_message(message, num, lg_message, num);

					if (send(sock, message, strlen(message), 0) < 0)
					{
						printf("Erreur send");
						exit(1);
					}

					afficher_message(message, lg_message, source, num);
				}
				shutdown(sock, 2);
				num = 0;
				printf("Source: fin !! \n");
			}

			else
			{
				printf("Echec de connexion");
				exit(1);
			}
		}

		// Recepteur
		else if (source == 0)
		{
			printf("Puits: lg_m_lu=%d, port=%d, n_receptions=%d, TD=TCP \n", lg_message, port, nb_message);

			if (sock == -1)
			{
				printf("Echec de creation du socket \n");
				exit(1);
			}

			printf("Puits: socket \n");

			// Criaçao do endereço de um socket
			memset((char *)&addr_local, 0, lg_addr_local);
			addr_local.sin_family = AF_INET;
			addr_local.sin_port = htons(port);
			addr_local.sin_addr.s_addr = INADDR_ANY;

			// Associaçao de um endereço ao socket
			if (bind(sock, (struct sockaddr *)&addr_local, lg_addr_local) == -1)
			{
				printf("Echec du bind.");
				exit(1);
			}

			lg_adr_client = sizeof(adr_client);
			int lg_recu = 1; // mensagens recebidas

			if (listen(sock, nb_message) == -1)
			{
				printf("Erreur listen");
				exit(1);
			}

			// Estabelecendo a conexão e criando o socket remoto
			if ((sock_b = accept(sock, (struct sockaddr *)&adr_client, &lg_adr_client)) == -1)
			{
				printf("Erreur du accept");
				exit(1);
			}

			printf("Puits: connexion acceptee avec %d \n", adr_client.sin_addr.s_addr);

			while (lg_recu > 0)
			{
				lg_recu = read(sock_b, message, lg_message);

				if (lg_recu < 0)
				{
					printf("Echec du read");
					exit(1);
				}
				if (lg_recu != 0)
				{
					num++;
					afficher_message(message, lg_recu, source, num);
				}
			}
			num = 0;

			if (shutdown(sock_b, 2) == -1)
			{
				printf("Echec du shotdown emetteur");
				exit(1);
			}
			if (close(sock_b) == -1)
			{
				printf("Echec destruction du socket");
				exit(1);
			}
			if (close(sock) == -1)
			{
				printf("Echec de ferture du socket");
				exit(1);
			}
			printf("Puits: fin!! \n");
		}
	}

	// BAL
	else if (boite == 1)
	{

		// Emetteur
		if (source == 1)
		{
			printf("Source: lg_msg_emis=%d, port=%d, m_envoie=%d, TP=TCP -> %s \n", lg_message, port, nb_message, dest);

			sock = creat_socket(udp);

			if (sock == -1)
			{
				printf("Echec de creation du socket \n");
				exit(1);
			}
			printf("Source: socket \n");

			// Criaçao do endereço do socket
			memset((char *)&adr_distant, 0, sizeof(adr_distant));
			addr_local.sin_family = AF_INET;
			addr_local.sin_port = htons(port);

			// Endereço IP
			if ((hp = gethostbyname(dest)) == NULL)
			{
				printf("erreur gethostbyname \n");
				exit(1);
			}
			memcpy((char *)&(adr_distant.sin_addr.s_addr),
				   hp->h_addr,
				   hp->h_length);

			if (connect(sock, (struct sockaddr *)&adr_distant, sizeof(adr_distant)) == 0)
			{
				printf("Source: connect \n");

				id.etat = source;
				id.lg_msg_bal = lg_message;
				id.nb_msg_bal = nb_message;

				if (send(sock, &id, sizeof(&id), 0) < 0)
				{
					printf("Erreur send");
					exit(1);
				}
				m_envoie = nb_message;

				while (m_envoie != 0)
				{
					m_envoie--;
					num++;

					construire_message(message, num, lg_message, receveur);

					if (send(sock, message, strlen(message), 0) < 0)
					{
						printf("Erreur send");
						exit(1);
					}

					aff_msg_bal(message, lg_message, source, num, receveur);
				}

				shutdown(sock, 2);
				num = 0;
				printf("Source: fin! \n");
			}

			else
			{
				printf("Echec de connexion");
				exit(1);
			}
		}

		// Recepteur
		printf("Puits: lg_m_lu=%d, port=%d, nb_receptions=%d, TP=udp \n", lg_message, port, nb_message);

		sock_local = creat_socket(udp);

		if (sock_local == -1)
		{
			printf("Echec de creation du socket local");
			exit(1);
		}

		printf("Source: socket \n");

		// Criaçao do endereço do socket local
		memset((char *)&addr_local, 0, sizeof(addr_local));
		addr_local.sin_family = AF_INET;
		addr_local.sin_port = /// ???????
			addr_local.sin_addr.s_addr = INADDR_ANY;

		// Associaçao do endereço do socket local
		if (bind(sock_local, (struct sockaddr *)&addr_local, sizeof(addr_local)) == -1)
		{
			printf("Echec du bind.");
			exit(1);
		}

		// Criaçao do endereço do socket distante
		memset((char *)&adr_distant, 0, sizeof(adr_distant));
		addr_local.sin_family = AF_INET;
		addr_local.sin_port = htons(port);

		// Atribuindo o endereço IP do soquete remoto
		if ((hp = gethostbyname(dest)) == NULL)
		{
			printf("erreur gethostbyname \n");
			exit(1);
		}
		memcpy((char *)&(adr_distant.sin_addr.s_addr),
			   hp->h_addr,
			   hp->h_length);

		if (connect(sock_local, (struct sockaddr *)&adr_distant, sizeof(adr_distant)) == 0)
		{
			printf("Source: Connect \n");

			if (send(sock_local, &source, sizeof(int), 0) < 0)
			{
				printf("Erreur send");
				exit(1);
			}

			if (listen(sock_local, nb_message) == -1)
			{
				printf("Erreur listen");
				exit(1);
			}

			/////// VOIR COMMENT RECUPERER LES DONNEES : longueur des messages à recevoir et nb de message à recevoir.
			/////// redimensionner la boucle while !!!!!

			while (lg_recu > 0)
			{
				lg_recu = read(sock_b, message, lg_message);

				if (lg_recu < 0)
				{
					printf("Echec du read");
					exit(1);
				}

				if (lg_recu != 0)
				{
					num++;
					aff_msg_bal(message, lg_message, source, num, receveur);
				}
			}
			num = 0;
		}
	}
	// BAL
	else if (source == 2)
	{
		printf("Puits: lg_m_lu=%d, port=%d, nb_receptions=%d, TP=udp \n ", lg_message, port, nb_message);

		sock_local = creat_socket(udp);

		if (sock_local == -1)
		{
			printf("Echec de creation du socket local");
			exit(1);
		}
		printf("Source: socket \n");

		// Criaçao do endereço do scoket local
		memset((char *)&addr_local, 0, sizeof(addr_local));
		addr_local.sin_family = AF_INET;
		addr_local.sin_port = htons(port);
		addr_local.sin_addr.s_addr = INADDR_ANY;

		// Associar o endereço ao socket
		if (bind(sock, (struct sockaddr *)&addr_local, lg_addr_local) == -1)
		{
			printf("Echec du bind");
			exit(1);
		}

		lg_adr_client = sizeof(adr_client);
		int lg_recu = 1;

		if (listen(sock_local, nb_message) == -1)
		{
			printf("Erreur listen");
			exit(1);
		}

		while (1)
		{

			if ((sock_b = accept(sock_local, (struct sockaddr *)&adr_client, &lg_adr_client)) == -1)
			{
				printf("Erreur du accept");
				exit(1);
			}
		}
	}
}
