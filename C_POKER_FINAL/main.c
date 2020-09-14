#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define DECK_SIZE 52
#define HAND_SIZE 2
#define TABLE_SIZE 5

#define FLOP_SIZE 3
#define TURN_SIZE 1
#define RIVER_SIZE 1

#define NAME_SIZE 20
#define SMALL_BLIND 10
#define BIG_BLIND 20

#define MIN_MONEY 100
#define MAX_MONEY 100000

#define AI_FOLD 0
#define AI_CHECK 0.25
#define AI_CALL 0.5
#define AI_RAISE 0.75
#define AI_FOLD_CANNOT_CHECK 0.4

const char* NAMES[9] =
{
	"AI1", "AI2", "AI3",
	"AI4", "AI5", "AI6",
	"AI7", "AI8", "AI9"
};

enum ERank {
	RANK_2,
	RANK_3,
	RANK_4,
	RANK_5,
	RANK_6,
	RANK_7,
	RANK_8,
	RANK_9,
	RANK_10,
	RANK_J,
	RANK_Q,
	RANK_K,
	RANK_A
};

typedef enum ERank Rank;

enum ESuit {
	SUIT_SPADE,
	SUIT_CLUB,
	SUIT_DIAMOND,
	SUIT_HEART
};

typedef enum ESuit Suit;

enum EChoice {
	CHOICE_FOLD,
	CHOICE_CHECK,
	CHOICE_CALL,
	CHOICE_RAISE,
	CHOICE_NOTHING
};

typedef enum EChoice Choice;

struct SCard {
	Rank rank;
	Suit suit;
};

typedef struct SCard Card;

struct SMove {
	int bet;
	Choice choice;
};

typedef struct SMove Move;

struct SPlayer {
	char name[NAME_SIZE]; //----
	int money;
	Card* cards[HAND_SIZE]; //staticeskyi massiv ukazatelei na karty
	Move move; // odin hod igroka
};

typedef struct SPlayer Player;

enum ECombinationType {
	FOLDED,
	HIGH_HAND,
	ONE_PAIR,
	TWO_PAIRS,
	THREE_OF_A_KIND,
	STRAIGHT,
	FLUSH,
	FULL_HOUSE,
	FOUR_OF_A_KIND,
	STRAIGHT_FLUSH,
	ROYAL_FLUSH
};

typedef enum ECombinationType CombinationType;

struct SCombination {
	CombinationType combinationType;
	Card* cards[5];
	int index; // Full house: 0 - 2+3 |  Two pairs: 0 - 2+2+1
			   //             1 - 3+2 |             1 - 1+2+2
			   //                                   2 - 2+1+2
};

typedef struct SCombination Combination;

Card deck[DECK_SIZE];  // Massiv koloda kart
int deckPosition;      // Poziciya karty, kotoraya lezhit sverhu kolody

int blindIndex;        // Index igroka na kotorom blind
int smallBlindIndex;   // Index igroka na kotorom malenkyi blind

int playerIndexMove;   // Index igroka kotoryi hodit
int maxBetPlayerIndex; // Index igroka, kotoryi sdelal samuyu vysokuyu stavku

int stopPlayerIndex;   // Index igroka na kotorom nuzhno zakonchit krug stavok

int playersNumber;

int bank;

Card* table[TABLE_SIZE]; // Karty (staticheskyi massiv ukazatelei na karty), kotorye na stole

Player* p1; //globalnaya premenaya
Player** players; //---- dynamicheskyi massiv ukazatelei na Player

void merge(Card** five_cards, int low, int mid, int high)
{
	int size1 = mid - low + 1;
	Card* L[5];

	for (int i = 0; i < size1; i++)
	{
		L[i] = five_cards[i + low];
	}

	int size2 = high - mid;
	Card* R[5];

	for (int i = 0; i < size2; i++)
	{
		R[i] = five_cards[i + mid + 1];
	}


	int l = 0, r = 0;
	int k = low;
	while (l < size1 && r < size2)
	{
		if (L[l]->rank < R[r]->rank)
		{
			five_cards[k++] = L[l];
			l++;
		}
		else
		{
			five_cards[k++] = R[r];
			r++;
		}
	}
	while (l < size1)
	{
		five_cards[k++] = L[l];
		l++;

	}
	while (r < size2)
	{
		five_cards[k++] = R[r];
		r++;
	}
}

void merge_sort(Card** five_cards, int low, int high)
{
	if (low < high)
	{
		int mid = low + (high - low) / 2;

		merge_sort(five_cards, low, mid);
		merge_sort(five_cards, mid + 1, high);
		merge(five_cards, low, mid, high);
	}
}

char* print_card(const Card* card)
{
	char rank[3];
	rank[1] = '\0';
	switch (card->rank)
	{

	case RANK_2:
		rank[0] = '2';
		break;
	case RANK_3:
		rank[0] = '3';
		break;
	case RANK_4:
		rank[0] = '4';
		break;
	case RANK_5:
		rank[0] = '5';
		break;
	case RANK_6:
		rank[0] = '6';
		break;
	case RANK_7:
		rank[0] = '7';
		break;
	case RANK_8:
		rank[0] = '8';
		break;
	case RANK_9:
		rank[0] = '9';
		break;
	case RANK_10:
		rank[0] = '1';
		rank[1] = '0';
		rank[2] = '\0';
		break;
	case RANK_J:
		rank[0] = 'J';
		break;
	case RANK_Q:
		rank[0] = 'Q';
		break;
	case RANK_K:
		rank[0] = 'K';
		break;
	case RANK_A:
		rank[0] = 'A';
		break;
	default:
		printf("Unexpected rank: %d", card->rank);
		exit(EXIT_FAILURE);
	}

	char* cardStr = (char*)malloc(sizeof(char) * 55);
	strcpy(cardStr, ".------.|");
	strcat(cardStr, rank);

	if (rank[0] == '1')
	{
		strcat(cardStr, "    |");
	}
	else
	{
		strcat(cardStr, "     |");
	}

	switch (card->suit)
	{
	case SUIT_SPADE:
		strcat(cardStr, "|  /\\  |"
			            "| (__) |");
		break;
	case SUIT_CLUB:
		strcat(cardStr, "|  ()  |"
			            "| ()() |");
		break;
	case SUIT_DIAMOND:
		strcat(cardStr, "|  /\\  |"
			            "|  \\/  |");
		break;
	case SUIT_HEART:
		strcat(cardStr, "| (\\/) |"
			            "|  \\/  |");
		break;
	default:
		printf("Unexpected suit: %d", card->suit);
		exit(EXIT_FAILURE);
	}

	strcat(cardStr, "|    ");

	if (rank[0] != '1')
	{
		strcat(cardStr, " ");
	}
	strcat(cardStr, rank);
	strcat(cardStr, "|'------'");

	strcat(cardStr, "\0");

	//printf("\n%s", cardStr);

	return cardStr;
}

void print_cards(Card** cards, int cardNumber)
{
	char** cardsStr = (char**)malloc(sizeof(char*) * cardNumber);

	int i;
	for (i = 0; i < cardNumber; ++i)
	{
		cardsStr[i] = print_card(cards[i]);
	}

	int y;
	for (y = 0; y < 6; y++)
	{
		for (i = 0; i < cardNumber; i++)
		{
			int x;
			for (x = 0; x < 8; x++)
			{
				printf("%c", cardsStr[i][y * 8 + x]);
			}
		}
		printf("\n");
	}

	for (i = 0; i < cardNumber; i++)
	{
		free(cardsStr[i]);
	}
	free(cardsStr);
}

void print_deck()
{
	int i;
	for (i = 0; i < DECK_SIZE; i++)
	{
		char* card = print_card(&deck[i]);
		int y;
		for (y = 0; y < 6; y++)
		{
			int x;
			for (x = 0; x < 8; x++)
			{
				printf("%c", card[y * 8 + x]);
			}
			printf("\n");
		}
	}
}

void shuffle_deck()
{
	int i;
	int j;
	Card temp;
	for (i = 0; i < DECK_SIZE; ++i)
	{
		j = rand() % DECK_SIZE;
		temp = deck[i];
		deck[i] = deck[j];
		deck[j] = temp;
	}
}

void initialize_deck()
{
	int i;
	int j;
	int k = 0;
	for (i = RANK_2; i <= RANK_A; ++i)
	{
		for (j = SUIT_SPADE; j <= SUIT_HEART; ++j)
		{
			deck[k].rank = i;
			deck[k].suit = j;
			k++;
		}
	}
}

void initialize_players(int playersNumber, int money)
{
	int i;
	for (i = 0; i < playersNumber; ++i)
	{
		players[i] = (Player*)malloc(sizeof(Player));
		strcpy(players[i]->name, NAMES[i]);
		players[i]->money = money;
	}

	players[playersNumber] = p1; //dobavlenie menya v posledni element massiva
	p1->money = money;
}

void print_player(Player* player, int withCards)
{
	printf("%s: %d$", player->name, player->money);

	if (player == p1 || withCards == 1)
	{
		printf("\n");

		print_cards(player->cards, HAND_SIZE);
	}
}

void print_players(int withCards)
{
	int i;
	for (i = 0; i < playersNumber; i++)
	{
		print_player(players[i], withCards);
		printf("\n");
	}
}

void deal_cards()
{
	int i;
	int j;
	for (i = 0; i < playersNumber; i++)
	{
		for (j = 0; j < HAND_SIZE; j++)
		{
			players[i]->cards[j] = &deck[deckPosition++];
		}
	}
}

void flushInput() {
	int c;
	while ((c = getchar()) != EOF && c != '\n'); // prosto ignoriruem vse znaky
}

int getIntegerInput(int min, int max) {
	int ret = 0;
	do
	{
		printf("Please enter a number in range: [%d-%d]\n", min, max);
		if (scanf("%d", &ret) != 1) {
			printf("Please enter one integer!\n");
			flushInput();
		}
	} while (!(ret >= min && ret <= max));
	return ret;
}

int can_check()
{
	//return players[playerIndexMove]->move.bet == players[maxBetPlayerIndex]->move.bet;
	if (players[playerIndexMove]->move.bet == players[maxBetPlayerIndex]->move.bet)
	{
		return 1;
	}

	return 0;
}

int can_call()
{
	if (players[playerIndexMove]->money >= players[maxBetPlayerIndex]->move.bet &&
		players[playerIndexMove]->move.bet != players[maxBetPlayerIndex]->move.bet)
	{
		return 1;
	}

	return 0;
}

int can_raise()
{
	if (players[playerIndexMove]->money > players[maxBetPlayerIndex]->move.bet)
	{
		return 1;
	}

	return 0;
}

void call_bet()
{
	players[playerIndexMove]->move.bet = players[maxBetPlayerIndex]->move.bet;
}

int is_correct_bet(int bet)
{
	if (players[playerIndexMove]->money >= bet && players[maxBetPlayerIndex]->move.bet < bet &&
		bet >= BIG_BLIND && players[playerIndexMove]->move.bet < bet)
	{
		return 1;
	}

	return 0;
}

void raise_bet()
{
	int bet;

	if (players[playerIndexMove] == p1)
	{
		printf("Enter your bet: ");

		do
		{
			if (scanf("%d", &bet) != 1)
			{
				flushInput();
				continue;
			}
			if (!is_correct_bet(bet)) {
				printf("Your bet is less or equal to max bet or you don't have enought money. Please enter your bet again: ");
				continue;
			}

			break;

		} while (1);
	}
	else
	{
		bet = BIG_BLIND * 2; // improve
		printf(" bets %d\n", bet);
	}

	players[playerIndexMove]->move.bet = bet;
	maxBetPlayerIndex = playerIndexMove;
	stopPlayerIndex = playerIndexMove;
}

void move_p1()
{
get_choice:
	printf("What you would like to do? (0-fold, 1-check, 2-call, 3-raise). "); //peredelat na %d, CHOICE_FOLD ....
	int choice = getIntegerInput(CHOICE_FOLD, CHOICE_RAISE);

	switch (choice)
	{
	case CHOICE_CHECK:
		if (!can_check())
		{
			printf("You cannot check because there is a raise already\n");
			goto get_choice;
		}
		break;

	case CHOICE_CALL:
		if (can_call())
		{
			call_bet();
		}
		else
		{
			printf("You cannot call because of lack of money or the bet is already called\n");
			goto get_choice;
		}
		break;

	case CHOICE_RAISE:
		if (can_raise())
		{
			raise_bet();
		}
		else
		{
			printf("You cannot call because of lack of money\n");
			goto get_choice;
		}
		break;

	default:
		break;
	}

	p1->move.choice = choice;
}

Choice move_value_to_choice(double res)
{
	if (res >= AI_FOLD && res < AI_CHECK)
	{
		return CHOICE_FOLD;
	}
	else if (res >= AI_CHECK && res < AI_CALL)
	{
	check:
		if (!can_check())
		{
			if (res >= AI_CHECK && res < AI_FOLD_CANNOT_CHECK)
			{
				return CHOICE_FOLD;
			}
			if (can_call()) return CHOICE_CALL;
			return CHOICE_FOLD;
		}

		return CHOICE_CHECK;
	}
	else if (res >= AI_CALL && res < AI_RAISE)
	{
	call:
		if (!can_call())
		{
			goto check;
		}
		return CHOICE_CALL;
	}
	else if (res >= AI_RAISE)
	{
		if (!can_raise())
		{
			goto call;
		}
		return CHOICE_RAISE;
	}

	return CHOICE_FOLD;
}

double ai_decision(Player* player)
{
	double res = 0;

	Card* card1 = player->cards[0];
	Card* card2 = player->cards[1];

	//if (table[0] == NULL)
	{
		if (card1->suit == card2->suit)
		{
			res += 0.1;
		}

		if (card1->rank == card2->rank)
		{
			if (card1->rank < RANK_8)
			{
				res += 0.3;
			}
			else
			{
				res += 0.5;
			}
		}
		else {
			int rankDiff = abs(card1->rank - card2->rank);
			if (rankDiff <= 4)
			{
				res += 0.2;
			}
		}

		if (card1->rank > RANK_10 || card2->rank > RANK_10)
		{
			res += 0.35;
		}
	}
	//else
	//{
	//	
	//}

	return res;
}

void print_choice(Choice choice)
{
	switch (choice)
	{
	case CHOICE_FOLD:
		printf("Fold");
		break;
	case CHOICE_CHECK:
		printf("Check");
		break;
	case CHOICE_CALL:
		printf("Call");
		break;
	case CHOICE_RAISE:
		printf("Increase");
		break;
	default:
		break;
	}
}

void move_ai()
{
	double res = ai_decision(players[playerIndexMove]);
	Choice choice = move_value_to_choice(res);

	switch (choice)
	{
	case CHOICE_CALL:
		call_bet();
		break;

	case CHOICE_RAISE:
		raise_bet();

		break;

	default:
		break;
	}

	players[playerIndexMove]->move.choice = choice;

	print_choice(choice);
	printf("\n");
}

void clear_table();

void new_game()
{
	printf("--------------------\n");
	shuffle_deck();
	deckPosition = 0;
	//print_deck();
	deal_cards();
	printf("Cards are dealt\n");

	print_players(0);

	blindIndex = (blindIndex + 1) % playersNumber;
	smallBlindIndex = (smallBlindIndex + 1) % playersNumber;

	maxBetPlayerIndex = blindIndex;
	playerIndexMove = (blindIndex + 1) % playersNumber;
	stopPlayerIndex = playerIndexMove;

	printf("Big Blind (%d) at player: %s\n", BIG_BLIND, players[blindIndex]->name);
	printf("Small blind (%d) at player: %s\n", SMALL_BLIND, players[smallBlindIndex]->name);

	int i;
	for (i = 0; i < playersNumber; i++)
	{
		players[i]->move.bet = 0;
		players[i]->move.choice = CHOICE_NOTHING;
	}

	players[blindIndex]->move.bet = BIG_BLIND;
	players[smallBlindIndex]->move.bet = SMALL_BLIND;

	clear_table();
}

int check_if_all_folded()
{
	// proveryaem esli vse igroki krome odnogo sbrosili
	int i;
	int foldedPlayers = 0;
	for (i = 0; i < playersNumber; i++) {
		if (players[i]->move.choice == CHOICE_FOLD) {
			foldedPlayers++;
		}
	}

	return foldedPlayers == playersNumber - 1;
}

int do_moves()
{
	int firstTurn = 1;

	while (playerIndexMove != stopPlayerIndex || firstTurn == 1)  // poka ne sravnyayutsyya stavki
	{
		firstTurn = 0;
		if (players[playerIndexMove] == p1 && p1->move.choice != CHOICE_FOLD)
		{
			printf("It's your move, %s. ", players[playerIndexMove]->name);
			move_p1();
		}
		else if (players[playerIndexMove]->move.choice != CHOICE_FOLD)
		{
			printf("Player %s ", players[playerIndexMove]->name);
			move_ai();
		}

		if (check_if_all_folded() == 1)
		{
			break;
		}

		playerIndexMove = (playerIndexMove + 1) % playersNumber; // sdvigaem na sled igroka (esli prevysit kol-vo igrokov ostatok po delenyuy vernet na 0)
	}

	int i;
	for (i = 0; i < playersNumber; i++)
	{
		bank += players[i]->move.bet;
		players[i]->money -= players[i]->move.bet;
		players[i]->move.bet = 0;
	}

	printf("BANK: %d\n", bank);

	if (check_if_all_folded() == 1)
	{
		for (i = 0; i < playersNumber; i++)
		{
			if (players[i]->move.choice != CHOICE_FOLD)
			{
				printf("All players folded, player %s won!\n", players[i]->name);
				players[i]->money += bank;
				bank = 0;
				return 0;
			}
		}
	}

	playerIndexMove = smallBlindIndex; // sled igrok kotoryi budet hodit'
	stopPlayerIndex = smallBlindIndex; // sled igrok na kotorom ostanovitsya cikl vverhu
	return 1;
}

void print_table()
{
	printf("TABLE: {\n");

	int i;
	int number = 0;
	for (i = 0; i < 5; i++)
	{
		if (table[i] != NULL)
		{
			number++;
		}
	}
	print_cards(table, number);

	printf("\n}\n");
}

void deal_to_table(int start, int end)
{
	deckPosition++; // skinut odnu kartu pered rasdachei
	int i;
	for (i = start; i < end; i++)
	{
		table[i] = &deck[deckPosition];
		deckPosition++;
	}
}

void deal_flop()
{
	deal_to_table(0, FLOP_SIZE);
	print_table();
}

void deal_turn()
{
	deal_to_table(FLOP_SIZE, FLOP_SIZE + TURN_SIZE);
	print_table();
}

void deal_river()
{
	deal_to_table(FLOP_SIZE + TURN_SIZE, FLOP_SIZE + TURN_SIZE + RIVER_SIZE);
	print_table();
}

int is_flush(Combination* combination)
{
	int c;
	Suit x = combination->cards[0]->suit;
	for (c = 1; c < 5; c++)
	{
		if (combination->cards[c]->suit != x)
		{
			return 0;
		}
	}
	return 1;
}

int is_straight(Combination* combination)
{
	int c;
	for (c = 0; c < 4; c++)
	{
		if (c == 3 && combination->cards[0]->rank == RANK_2 && combination->cards[c + 1]->rank == RANK_A)
		{
			return 1;
		}
		if (combination->cards[c]->rank + 1 != combination->cards[c + 1]->rank)
		{
			return 0;
		}
	}
	return 1;
}

int is_royal_flush(Combination* combination)
{
	int r;
	int c;
	r = RANK_10;

	for (c = 0; c < 5; c++)
	{
		if (combination->cards[c]->rank != r)
			return 0;
		r++;
	}

	return is_flush(combination);
}

int is_straight_flush(Combination* combination)
{
	return is_straight(combination) && is_flush(combination);
}

int is_four_of_a_kind(Combination* combination)
{
	int c;
	int fourOfKind = 1;

	combination->index = 0;

	for (c = 0; c < 3; c++)
	{
		if (combination->cards[c]->rank != combination->cards[c + 1]->rank)
		{
			fourOfKind = 0;
			break;
		}
	}

	if (fourOfKind == 1)
	{
		return 1;
	}

	fourOfKind = 1;
	combination->index = 1;

	for (c = 1; c < 4; c++)
	{
		if (combination->cards[c]->rank != combination->cards[c + 1]->rank)
		{
			fourOfKind = 0;
			break;
		}
	}

	return fourOfKind;
}

int is_full_house(Combination* combination)
{
	// 2+3
	if (combination->cards[0]->rank == combination->cards[1]->rank &&
		combination->cards[2]->rank == combination->cards[3]->rank && combination->cards[3]->rank == combination->cards[4]->rank)
	{
		combination->index = 0;
		return 1;
	}

	// 3+2
	if (combination->cards[0]->rank == combination->cards[1]->rank && combination->cards[1]->rank == combination->cards[2]->rank &&
		combination->cards[3]->rank == combination->cards[4]->rank)
	{
		combination->index = 1;
		return 1;
	}

	return 0;
}

int is_three_of_a_kind(Combination* combination)
{
	int c;

	for (c = 0; c < 3; c++)
	{
		if (combination->cards[c]->rank == combination->cards[c + 1]->rank && combination->cards[c]->rank == combination->cards[c + 2]->rank)
		{
			combination->index = c;
			return 1;
		}
	}
	return 0;
}

int is_two_pairs(Combination* combination)
{
	if (combination->cards[0]->rank == combination->cards[1]->rank &&
		combination->cards[2]->rank == combination->cards[3]->rank)
	{
		combination->index = 0;
		return 1;
	}

	if (combination->cards[1]->rank == combination->cards[2]->rank &&
		combination->cards[3]->rank == combination->cards[4]->rank)
	{
		combination->index = 1;
		return 1;
	}

	if (combination->cards[0]->rank == combination->cards[1]->rank &&
		combination->cards[3]->rank == combination->cards[4]->rank)
	{
		combination->index = 2;
		return 1;
	}

	return 0;
}

int is_one_pair(Combination* combination)
{
	int c;

	for (c = 0; c < 4; c++)
	{
		if (combination->cards[c]->rank == combination->cards[c + 1]->rank)
		{
			combination->index = c;
			return 1;
		}
	}
	return 0;
}

int high_hand(Combination* combination)
{
	combination->index = 4;
	return 1;
}

void find_combination(Combination* combination)
{
	merge_sort(combination->cards, 0, 4);

	if (is_royal_flush(combination) == 1)
	{
		combination->combinationType = ROYAL_FLUSH;
		return;
	}

	if (is_straight_flush(combination) == 1)
	{
		combination->combinationType = STRAIGHT_FLUSH;
		return;
	}

	if (is_four_of_a_kind(combination) == 1)
	{
		combination->combinationType = FOUR_OF_A_KIND;
		return;
	}

	if (is_full_house(combination) == 1)
	{
		combination->combinationType = FULL_HOUSE;
		return;
	}

	if (is_flush(combination) == 1)
	{
		combination->combinationType = FLUSH;
		return;
	}

	if (is_straight(combination) == 1)
	{
		combination->combinationType = STRAIGHT;
		return;
	}

	if (is_three_of_a_kind(combination) == 1)
	{
		combination->combinationType = THREE_OF_A_KIND;
		return;
	}

	if (is_two_pairs(combination) == 1)
	{
		combination->combinationType = TWO_PAIRS;
		return;
	}

	if (is_one_pair(combination) == 1)
	{
		combination->combinationType = ONE_PAIR;
		return;
	}

	high_hand(combination);
	combination->combinationType = HIGH_HAND;
}

int compare_combinations(Combination* left, Combination* right); // deklaraciya funkcii

void find_max_combination(Player* player, Combination* maxCombination) // sostavlyet vse vozmozhnye kombinacii
{
	Card* cards[HAND_SIZE + TABLE_SIZE];

	int i;
	for (i = 0; i < TABLE_SIZE; i++)
	{
		cards[i] = table[i];
	}

	for (i = 0; i < HAND_SIZE; i++)
	{
		cards[TABLE_SIZE + i] = player->cards[i];
	}

	int firstCombination = 1;

	int j, k, l, m;
	for (i = 0; i < TABLE_SIZE + HAND_SIZE; i++)
	{
		for (j = i + 1; j < TABLE_SIZE + HAND_SIZE; j++)
		{
			for (k = j + 1; k < TABLE_SIZE + HAND_SIZE; k++)
			{
				for (l = k + 1; l < TABLE_SIZE + HAND_SIZE; l++)
				{
					for (m = l + 1; m < TABLE_SIZE + HAND_SIZE; m++)
					{
						Combination combination;
						combination.cards[0] = cards[i];
						combination.cards[1] = cards[j];
						combination.cards[2] = cards[k];
						combination.cards[3] = cards[l];
						combination.cards[4] = cards[m];

						find_combination(&combination); // dlya 5 kart nachodit kombinaciu

						if (firstCombination == 1 || compare_combinations(maxCombination, &combination) < 0)
						{
							firstCombination = 0;

							maxCombination->combinationType = combination.combinationType;
							maxCombination->index = combination.index;

							int y;
							for (y = 0; y < 5; y++)
							{
								maxCombination->cards[y] = combination.cards[y];
							}
						}
					}
				}
			}
		}
	}
}

void clear_table()
{
	int i;

	for (i = 0; i < TABLE_SIZE; i++)
	{
		table[i] = NULL;
	}
}

int compare_by_high_hand(Combination* left, Combination* right) // oshibka
{
	int i;
	for (i = 4; i > 0; i--)
	{
		if (left->cards[i]->rank > right->cards[i]->rank)
		{
			return 1;
		}
		else if (right->cards[i]->rank > left->cards[i]->rank)
		{
			return -1;
		}
	}
	return 0;
}

int compare_straight(Combination* left, Combination* right)
{
	if (left->cards[0]->rank > right->cards[0]->rank)
	{
		return 1;
	}
	if (left->cards[0]->rank < right->cards[0]->rank)
	{
		return -1;
	}
	if (left->cards[0]->rank == right->cards[0]->rank)
	{
		if (left->cards[4]->rank != RANK_A && right->cards[4]->rank == RANK_A)
		{
			return 1;
		}
		if (left->cards[4]->rank == RANK_A && right->cards[4]->rank != RANK_A)
		{
			return -1;
		}

		if (left->cards[4]->rank > right->cards[4]->rank)
		{
			return 1;
		}
		if (left->cards[4]->rank < right->cards[4]->rank)
		{
			return -1;
		}
	}

	return 0;
}

int compare_combinations(Combination* left, Combination* right) // sravnit dve odinakovye po ranku kombinacii (naprimer comb_type == one pairs)
{
	if (left->combinationType > right->combinationType)
	{
		return 1;
	}
	if (left->combinationType < right->combinationType)
	{
		return -1;
	}

	if (left->combinationType == FOLDED)
		return 0;

	//HIGH_HAND
	if (left->combinationType == HIGH_HAND)
	{
		return compare_by_high_hand(left, right);
	}

	//ONE_PAIR
	if (left->combinationType == ONE_PAIR)
	{
		if (left->cards[left->index]->rank > right->cards[right->index]->rank)
		{
			return 1;
		}
		if (left->cards[left->index]->rank < right->cards[right->index]->rank)
		{
			return -1;
		}
		return compare_by_high_hand(left, right);
	}

	//TWO_PAIRS
	if (left->combinationType == TWO_PAIRS)
	{
		int firstIndexLeft = left->index;
		int secondIndexLeft = firstIndexLeft + 2;

		if (firstIndexLeft == 2)
		{
			firstIndexLeft = 0;
		}

		int firstIndexRight = right->index;
		int secondIndexRight = firstIndexRight + 2;

		if (firstIndexRight == 2)
		{
			firstIndexRight = 0;
		}

		if (left->cards[secondIndexLeft]->rank > right->cards[secondIndexRight]->rank)
		{
			return 1;
		}
		if (left->cards[secondIndexLeft]->rank < right->cards[secondIndexRight]->rank)
		{
			return -1;
		}

		if (left->cards[firstIndexLeft]->rank > right->cards[firstIndexRight]->rank)
		{
			return 1;
		}
		if (left->cards[firstIndexLeft]->rank < right->cards[firstIndexRight]->rank)
		{
			return -1;
		}
		return compare_by_high_hand(left, right);
	}

	//THREE_OF_A_KIND
	if (left->combinationType == THREE_OF_A_KIND)
	{
		if (left->cards[left->index]->rank > right->cards[right->index]->rank)
		{
			return 1;
		}
		if (left->cards[left->index]->rank < right->cards[right->index]->rank)
		{
			return -1;
		}
		return compare_by_high_hand(left, right);
	}

	//STRAIGHT
	if (left->combinationType == STRAIGHT)
	{
		return compare_straight(left, right);
	}

	//FLUSH
	if (left->combinationType == FLUSH)
	{
		return compare_by_high_hand(left, right);
	}

	//FULL_HOUSE
	if (left->combinationType == FULL_HOUSE)
	{
		int pairIndexLeft;  // 2
		int threeOfKindIndexLeft = 0; // + 3

		if (left->index == 0)
		{
			pairIndexLeft = 0;
			threeOfKindIndexLeft = 2;
		}
		else if (left->index == 1)
		{
			pairIndexLeft = 3;
			threeOfKindIndexLeft = 0;
		}

		int pairIndexRight;  // 2
		int threeOfKindIndexRight = 0; // + 3

		if (right->index == 0)
		{
			pairIndexRight = 0;
			threeOfKindIndexRight = 2;
		}
		else if (right->index == 1)
		{
			pairIndexRight = 3;
			threeOfKindIndexRight = 0;
		}

		if (left->cards[threeOfKindIndexLeft]->rank > right->cards[threeOfKindIndexRight]->rank)
		{
			return 1;
		}
		if (left->cards[threeOfKindIndexLeft]->rank < right->cards[threeOfKindIndexRight]->rank)
		{
			return -1;
		}
		if (left->cards[pairIndexLeft]->rank > right->cards[pairIndexRight]->rank)
		{
			return 1;
		}
		if (left->cards[pairIndexLeft]->rank < right->cards[pairIndexRight]->rank)
		{
			return -1;
		}
		return 0;
	}

	//FOUR_OF_A_KIND
	if (left->combinationType == FOUR_OF_A_KIND)
	{
		if (left->cards[left->index]->rank > right->cards[right->index]->rank)
		{
			return 1;
		}
		if (left->cards[left->index]->rank < right->cards[right->index]->rank)
		{
			return -1;
		}

		int kickerIndexLeft = 0;
		if (left->index == 0)
		{
			kickerIndexLeft = 4;
		}
		else if (left->index == 1)
		{
			kickerIndexLeft = 0;
		}

		int kickerIndexRight = 0;
		if (right->index == 0)
		{
			kickerIndexRight = 4;
		}
		else if (right->index == 1)
		{
			kickerIndexRight = 0;
		}

		if (left->cards[kickerIndexLeft]->rank > right->cards[kickerIndexRight]->rank)
		{
			return 1;
		}
		if (left->cards[kickerIndexLeft]->rank < right->cards[kickerIndexRight]->rank)
		{
			return -1;
		}
		return 0;
	}

	// STRAIGHT_FLUSH
	if (left->combinationType == STRAIGHT_FLUSH)
	{
		return compare_straight(left, right);
	}

	return compare_by_high_hand(left, right); //ROYAL_FLUSH
}

void print_combination_type(CombinationType combinationType)
{
	switch (combinationType)
	{
	case HIGH_HAND:
		printf("High hand: ");
		break;
	case ONE_PAIR:
		printf("One pair: ");
		break;
	case TWO_PAIRS:
		printf("Two pairs: ");
		break;
	case THREE_OF_A_KIND:
		printf("Three of a kind: ");
		break;
	case STRAIGHT:
		printf("Straight: ");
		break;
	case FLUSH:
		printf("Flush: ");
		break;
	case FULL_HOUSE:
		printf("Full house: ");
		break;
	case FOUR_OF_A_KIND:
		printf("Four of a kind: ");
		break;
	case STRAIGHT_FLUSH:
		printf("Straight flush: ");
		break;
	case ROYAL_FLUSH:
		printf("Royal flush: ");
		break;
	default:
		break;
	}

	printf("\n");
}

void print_combination(Combination* combination)
{
	print_combination_type(combination->combinationType);
	print_cards(combination->cards, 5);
}

void find_winners()
{
	int i;

	Combination* combinations = (Combination*)malloc(sizeof(Combination) * (playersNumber)); //massiv kombinacyi vseh igrokov

	for (i = 0; i < playersNumber; i++)
	{
		if (players[i]->move.choice != CHOICE_FOLD)
		{
			find_max_combination(players[i], &combinations[i]);

			printf("player %s has:\n", players[i]->name);
			print_combination_type(combinations[i].combinationType);
			print_cards(players[i]->cards, HAND_SIZE);
			printf("\n");
		}
		else
		{
			combinations[i].combinationType = FOLDED;
		}
	}

	Combination* maxCombination = &combinations[0];
	for (i = 1; i < playersNumber; i++)
	{
		if (compare_combinations(maxCombination, &combinations[i]) < 0)
		{
			maxCombination = &combinations[i];
		}
	}

	Player** winners = (Player**)malloc(sizeof(Player*) * (playersNumber)); //massiv igrokov kotorye vyigrayut
	int k = 0;

	for (i = 0; i < playersNumber; i++)
	{
		if (compare_combinations(maxCombination, &combinations[i]) == 0)
		{
			winners[k] = players[i];
			k++;
		}
	}

	printf("Winners:\n");
	printf("--------------------\n");
	for (i = 0; i < k; i++)
	{
		printf("player %s has:\n", winners[i]->name);
		print_combination(maxCombination);
		printf("\nwon: %d\n", bank / k);

		winners[i]->money += bank / k;
	}
	printf("--------------------\n");

	bank = 0;

	free(winners);
	free(combinations);
}

int game_over()
{
	if (p1->money <= 0)
	{
		return 1;
	}

	int i;
	for (i = 0; i < playersNumber; i++)
	{
		if (players[i]->money <= 0)
		{
			free(players[i]);

			int j;
			for (j = i; j < playersNumber - 1; j++)
			{
				players[j] = players[j + 1];
			}

			playersNumber--;
		}
	}

	return playersNumber == 1;
}

int main(int argc, char** argv)
{
	srand(time(NULL)); //dlya random

	p1 = (Player*)malloc(sizeof(Player));

	printf("What's your name?\n");
	scanf("%s", p1->name);

	printf("Enter amount of start money\n");
	int money = getIntegerInput(MIN_MONEY, MAX_MONEY);

	printf("How many AI will be playing?\n");
	playersNumber = getIntegerInput(1, 9);

	players = (Player**)malloc(sizeof(Player*) * (playersNumber + 1)); //massiv igrokov

	smallBlindIndex = playersNumber;

	initialize_deck();
	initialize_players(playersNumber, money);

	playersNumber++;

	printf("----GAME STARTED----\n");

	while (!game_over())
	{
		new_game();

		if (!do_moves())
		{
			continue;
		}
		deal_flop();

		if (!do_moves())
		{
			continue;
		}
		deal_turn();

		if (!do_moves())
		{
			continue;
		}
		deal_river();

		if (!do_moves())
		{
			continue;
		}
		find_winners();
		printf("--------------------\n");
	}

	printf("-----GAME ENDED-----\n");

	int i;
	for (i = 0; i < playersNumber; ++i)
	{
		free(players[i]);
	}
	free(players);

	system("pause");
	return 0;
}
