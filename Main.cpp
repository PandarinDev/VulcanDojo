#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <limits>
#include <memory>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <type_traits>
#include <unordered_map>

// Type definitions

using byte = std::int8_t;
using ubyte = std::uint8_t;

// Constants

static constexpr float MATH_PI = 3.14159265358979323846f;
static constexpr ubyte BOARD_SIZE = 6;
static constexpr ubyte HAND_SIZE = 8;
static constexpr ubyte MAX_CARDS = 2 * BOARD_SIZE + HAND_SIZE;
static constexpr long long MILLIS_PER_TURN = 100;
static constexpr long long TREE_BUILDING_MAX_MILLIS = MILLIS_PER_TURN - 10;
static constexpr float CURVE_MEAN = 3.0f;
static constexpr float CURVE_VARIANCE = 1.4f;
static constexpr float CURVE_VALUE = 5.0f;

// Card stat factors for weighting

static constexpr float ATTACK_FACTOR = 1.0f;
static constexpr float DEFENSE_FACTOR = 0.8f;
static constexpr float PLAYER_HP_FACTOR = 0.8f;
static constexpr float OPPONENT_HP_FACTOR = 1.0f;
static constexpr float CARD_DRAW_FACTOR = 0.5f;

// Card ability factors for weighting

static constexpr float BREAKTHROUGH_FACTOR = 0.15f;
static constexpr float CHARGE_FACTOR = 0.5f;
static constexpr float DRAIN_FACTOR = 0.075f;
static constexpr float GUARD_FACTOR = 0.3f;
static constexpr float LETHAL_FACTOR = 2.0f;
static constexpr float WARD_FACTOR = 1.0f;

// General utility functions

template<typename T>
auto enum_value(T e) {
	return static_cast<std::underlying_type_t<T>>(e);
};

auto current_time_in_millis() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();
}

template<typename T, typename P>
auto max(T&& collection, P predicate) {
	return std::max_element(collection.begin(), collection.end(), predicate);
}

template<typename T, typename P>
auto min(T&& collection, P predicate) {
	return std::min_element(collection.begin(), collection.end(), predicate);
}

ubyte add_ability(ubyte abilities, ubyte ability) {
	return abilities | ability;
}

ubyte remove_ability(ubyte abilities, ubyte ability) {
	return abilities & ~ability;
}

bool has_ability(ubyte abilities, ubyte ability) {
	return (abilities & ability) != 0;
}

// Data models

struct Player {

	byte health;
	ubyte mana;
	ubyte deck;
	ubyte runes;

	Player(byte health, ubyte mana, ubyte deck, ubyte runes)
		: health(health), mana(mana), deck(deck), runes(runes) {}

	void spendMana(ubyte mana) {
		this->mana -= mana;
	}

};

enum class CardLocation : byte {
	HAND = 0,
	MY_SIDE = 1,
	OPPONENT_SIDE = -1
};

enum class CardType : ubyte {
	CREATURE = 0,
	GREEN_ITEM = 1,
	RED_ITEM = 2,
	BLUE_ITEM = 3
};

enum class CardAbility {
	EMPTY = 0b000000,
	BREAKTHROUGH = 0b000001,
	CHARGE = 0b000010,
	GUARD = 0b000100,
	DRAIN = 0b001000,
	LETHAL = 0b010000,
	WARD = 0b100000
};

struct Card {

	ubyte number;
	ubyte instanceId;
	CardLocation location;
	CardType type;
	ubyte cost;
	byte attack;
	byte defense;
	ubyte abilities;
	byte myHealthChange;
	byte opponentHealthChange;
	ubyte cardDraw;
	bool playedThisTurn;
	bool alreadyAttacked;

	Card(
		ubyte number, ubyte instanceId, CardLocation location,
		CardType type, ubyte cost, byte attack, byte defense,
		ubyte abilities, byte myHealthChange, byte opponentHealthChange,
		ubyte cardDraw, bool playedThisTurn, bool alreadyAttacked)
		: number(number), instanceId(instanceId), location(location),
		type(type), cost(cost), attack(attack), defense(defense), abilities(abilities),
		myHealthChange(myHealthChange), opponentHealthChange(opponentHealthChange),
		cardDraw(cardDraw), playedThisTurn(playedThisTurn), alreadyAttacked(alreadyAttacked) {}

	bool canAttack() const {
		return !alreadyAttacked && (!playedThisTurn || hasAbility(CardAbility::CHARGE));
	}

	bool hasAbility(CardAbility ability) const {
		return has_ability(abilities, enum_value(ability));
	}

	Card addAbility(CardAbility ability) const {
		return Card(
			number, instanceId, location, type, cost, attack, defense,
			add_ability(abilities, enum_value(ability)), myHealthChange,
			opponentHealthChange, cardDraw, playedThisTurn, alreadyAttacked
		);
	}

	void removeAbility(CardAbility ability) {
		abilities = remove_ability(abilities, enum_value(ability));
	}

	Card changeLocation(CardLocation location) const {
		return Card(
			number, instanceId, location, type, cost, attack, defense,
			abilities, myHealthChange, opponentHealthChange, cardDraw,
			playedThisTurn, alreadyAttacked);
	}

	void play() {
		location = CardLocation::MY_SIDE;
		playedThisTurn = true;
	}

	void flagAlreadyAttacked() {
		alreadyAttacked = true;
	}

	bool operator==(const Card& other) const {
		return
			number == other.number &&
			instanceId == other.instanceId &&
			location == other.location &&
			type == other.type &&
			cost == other.cost &&
			attack == other.attack &&
			defense == other.defense &&
			abilities == other.abilities &&
			myHealthChange == other.myHealthChange &&
			opponentHealthChange == other.opponentHealthChange &&
			cardDraw == other.cardDraw &&
			playedThisTurn == other.playedThisTurn &&
			alreadyAttacked == other.alreadyAttacked;
	}

	bool operator!=(const Card& other) const {
		return !((*this) == other);
	}

};

std::ostream& operator<<(std::ostream& os, const Card& card) {
	os << std::to_string(card.number) << " " << std::to_string(card.instanceId) << " ";
	os << std::to_string(enum_value(card.location)) << " " << std::to_string(enum_value(card.type)) << " ";
	os << std::to_string(card.cost) << " " << std::to_string(card.attack) << " ";
	os << std::to_string(card.defense) << " " << std::to_string(card.abilities) << std::to_string(card.myHealthChange) << " ";
	os << std::to_string(card.opponentHealthChange) << " " << std::to_string(card.cardDraw);
	return os;
}

struct GameState {

	Player me;
	Player opponent;
	ubyte opponentHand;
	ubyte cardCount;
	std::vector<Card> cards;

	template<typename Cards>
	GameState(Player me, Player opponent, ubyte opponentHand, ubyte cardCount, Cards&& cards)
		: me(me), opponent(opponent), opponentHand(opponentHand), cardCount(cardCount), cards(std::forward<Cards>(cards)) {}

	bool isDraftingPhase() const {
		return me.mana == 0;
	}

	bool isLethal() const {
		return opponent.health <= 0;
	}

	bool hasLost() const {
		return me.health <= 0;
	}

};

struct Action {
	virtual ~Action() = default;
	virtual std::string getCommand() const = 0;
	virtual GameState getResult(const GameState& state) const = 0;
};

struct PassAction final : public Action {

	std::string getCommand() const override {
		return "PASS";
	}

	GameState getResult(const GameState& state) const override {
		return state;
	}

};

struct PickAction final : public Action {

	const ubyte cardIndex;

	PickAction(ubyte cardIndex) : cardIndex(cardIndex) {}

	std::string getCommand() const override {
		return "PICK " + std::to_string(cardIndex);
	}

	GameState getResult(const GameState& state) const override {
		return state;
	}

};

struct SummonAction final : public Action {

	const Card card;

	SummonAction(const Card& card) : card(card) {}

	std::string getCommand() const override {
		return "SUMMON " + std::to_string(card.instanceId);
	}

	GameState getResult(const GameState& state) const override {
		Player newMe(state.me);
		newMe.health += card.myHealthChange;
		newMe.spendMana(card.cost);
		Player newOpponent(state.opponent);
		newOpponent.health += card.opponentHealthChange;
		std::vector<Card> copiedCards(state.cards);
		std::find(copiedCards.begin(), copiedCards.end(), card)->play();
		return GameState(newMe, newOpponent, state.opponentHand, state.cardCount, std::move(copiedCards));
	}

};

struct UseGreenItemAction final : public Action {

	const Card card;
	const Card target;

	UseGreenItemAction(const Card& card, const Card& target)
		: card(card), target(target) {}

	std::string getCommand() const override {
		return "USE " + std::to_string(card.instanceId) + " " + std::to_string(target.instanceId);
	}

	GameState getResult(const GameState& state) const override {
		// Heal me and decrease my mana
		Player newMe(
			state.me.health + card.myHealthChange,
			state.me.mana - card.cost,
			state.me.deck,
			state.me.runes);
		std::vector<Card> copiedCards(state.cards);
		// Possibly add new abilities to the target and increase it's attack and defense
		auto targetIt = std::find(copiedCards.begin(), copiedCards.end(), target);
		targetIt->abilities |= card.abilities;
		targetIt->attack += card.attack;
		targetIt->defense += card.defense;
		// Remove the item from the list of cards
		copiedCards.erase(std::remove(copiedCards.begin(), copiedCards.end(), card));
		return GameState(newMe, state.opponent, state.opponentHand, state.cardCount - 1, copiedCards);
	}

};

struct UseRedItemAction final : public Action {

	const Card card;
	const Card target;

	UseRedItemAction(const Card& card, const Card& target)
		: card(card), target(target) {}

	std::string getCommand() const override {
		return "USE " + std::to_string(card.instanceId) + " " + std::to_string(target.instanceId);
	}

	GameState getResult(const GameState& state) const override {
		Player newMe(state.me);
		newMe.spendMana(card.cost);
		// Damage opponent
		Player newOpponent(
			state.opponent.health + card.opponentHealthChange,
			state.opponent.mana,
			state.opponent.deck,
			state.opponent.runes
		);
		std::vector<Card> copiedCards(state.cards);
		// First remove the abilities of the target then calculate new attack and defense values
		// Note: Red items have negative attack and defense values.
		int newAbilities = target.abilities & ~card.abilities;
		int newAttack = target.attack + card.attack;
		int newDefense = target.defense + card.defense;
		int newCardCount = state.cardCount - 1;
		bool targetDied = false;
		if (card.defense < 0) {
			if (has_ability(newAbilities, enum_value(CardAbility::WARD))) {
				// Remove the ward, the creature is unharmed
				newAbilities = remove_ability(newAbilities, enum_value(CardAbility::WARD));
				newDefense = target.defense;
			}
			else if (card.defense <= -target.defense) {
				targetDied = true;
			}
		}
		if (targetDied) {
			// The creature dies, remove from the list of cards
			newCardCount -= 1;
			copiedCards.erase(std::remove(copiedCards.begin(), copiedCards.end(), target));
		}
		else {
			// Change the creature's stats
			auto targetIt = std::find(copiedCards.begin(), copiedCards.end(), target);
			targetIt->abilities = newAbilities;
			targetIt->attack = newAttack;
			targetIt->defense = newDefense;
		}
		// Remove the item from the list of cards
		copiedCards.erase(std::remove(copiedCards.begin(), copiedCards.end(), card));
		return GameState(newMe, newOpponent, state.opponentHand, newCardCount, copiedCards);
	}

};

struct UseBlueItemAction final : public Action {

	const Card card;
	const Card* target;

	UseBlueItemAction(const Card& card, const Card* target)
		: card(card), target(target) {}

	~UseBlueItemAction() {
		delete target;
	}

	std::string getCommand() const override {
		if (target == nullptr) {
			return "USE " + std::to_string(card.instanceId) + " -1";
		}
		return "USE " + std::to_string(card.instanceId) + " " + std::to_string(target->instanceId);
	}

	GameState getResult(const GameState& state) const override {
		// Positive effects are applied on us, negative effects applied on opponent
		Player newMe(state.me.health + card.myHealthChange, state.me.mana - card.cost, state.me.deck, state.me.runes);
		int opponentHealthChange = target != nullptr
			? card.opponentHealthChange
			: card.opponentHealthChange + card.defense;
		Player newOpponent(state.opponent.health + opponentHealthChange, state.opponent.mana, state.opponent.deck, state.opponent.runes);
		ubyte newCardCount = state.cardCount - 1;
		// Apply negative effects on creature if it was targeted
		if (target != nullptr) {
			std::vector<Card> copiedCards(state.cards);
			auto targetIt = std::find(copiedCards.begin(), copiedCards.end(), *target);
			if (target->hasAbility(CardAbility::WARD)) {
				// Remove the ward
				targetIt->removeAbility(CardAbility::WARD);
			}
			else if (-card.defense >= target->defense) {
				// The target is dead
				copiedCards.erase(targetIt);
				newCardCount -= 1;
			}
			else {
				// The target is damaged
				targetIt->defense += card.defense;
			}
			return GameState(newMe, newOpponent, state.opponentHand, newCardCount, copiedCards);
		}
		return GameState(newMe, newOpponent, state.opponentHand, newCardCount, state.cards);
	}

};

struct AttackFaceAction final : public Action {

	const Card card;

	AttackFaceAction(const Card& card) : card(card) {}

	std::string getCommand() const override {
		return "ATTACK " + std::to_string(card.instanceId) + " -1";
	}

	GameState getResult(const GameState& state) const override {
		Player newOpponent(state.opponent.health - card.attack,
			state.opponent.mana, state.opponent.deck, state.opponent.runes);
		std::vector<Card> copiedCards(state.cards);
		std::find(copiedCards.begin(), copiedCards.end(), card)->flagAlreadyAttacked();
		return GameState(state.me, newOpponent, state.opponentHand, state.cardCount, copiedCards);
	}

};

struct AttackCreatureAction final : public Action {

	const Card attacker;
	const Card target;

	AttackCreatureAction(const Card& attacker, const Card& target)
		: attacker(attacker), target(target) {}

	std::string getCommand() const override {
		return "ATTACK " +
			std::to_string(attacker.instanceId) +
			" " +
			std::to_string(target.instanceId);
	}

	GameState getResult(const GameState& state) const override {
		std::vector<Card> copiedCards(state.cards);
		auto attackerIt = std::find(copiedCards.begin(), copiedCards.end(), attacker);
		auto targetIt = std::find(copiedCards.begin(), copiedCards.end(), target);
		ubyte myNewHealth = state.me.health;
		ubyte opponentNewHealth = state.opponent.health;
		byte targetNewDefense = target.defense - attacker.attack;
		ubyte targetNewAbilities = target.abilities;
		ubyte attackerDealtDamage = 0;
		bool targetDied = false;
		// Handle the attacker -> target phase
		if (target.hasAbility(CardAbility::WARD)) {
			// The target does not receive any damage but loses ward
			targetNewDefense = target.defense;
			targetNewAbilities = remove_ability(target.abilities, enum_value(CardAbility::WARD));
		}
		else {
			attackerDealtDamage = std::min(attacker.attack, target.defense);
			if (attacker.hasAbility(CardAbility::BREAKTHROUGH) && attacker.attack > target.defense) {
				ubyte breakthroughDamage = attacker.attack - target.defense;
				opponentNewHealth -= breakthroughDamage;
			}
			if (attacker.attack >= target.defense || attacker.hasAbility(CardAbility::LETHAL)) {
				// The target dies
				targetDied = true;
				copiedCards.erase(std::remove(copiedCards.begin(), copiedCards.end(), target));
			}
		}
		if (!targetDied) {
			// The target receives damage
			targetIt->abilities = targetNewAbilities;
			targetIt->defense = targetNewDefense;
		}
		if (attacker.hasAbility(CardAbility::DRAIN)) {
			myNewHealth += attackerDealtDamage;
		}
		// Handle the target -> attacker phase
		byte attackerNewDefense = attacker.defense - target.attack;
		ubyte attackerNewAbilities = attacker.abilities;
		ubyte targetDealtDamage = 0;
		bool attackerDied = false;
		if (attacker.hasAbility(CardAbility::WARD)) {
			// The attacker does not receive any damage but loses ward
			attackerNewDefense = attacker.defense;
			attackerNewAbilities = remove_ability(attackerNewAbilities, enum_value(CardAbility::WARD));
		}
		else {
			targetDealtDamage = std::min(target.attack, attacker.defense);
			if (target.hasAbility(CardAbility::BREAKTHROUGH) && target.attack > attacker.defense) {
				ubyte breakthroughDamage = target.attack - attacker.defense;
				myNewHealth -= breakthroughDamage;
			}
			if (target.attack >= attacker.defense || target.hasAbility(CardAbility::LETHAL)) {
				// The attacker dies
				attackerDied = true;
				copiedCards.erase(std::remove(copiedCards.begin(), copiedCards.end(), attacker));
			}
		}
		if (!attackerDied) {
			// The attacker receives damage
			attackerIt->abilities = attackerNewAbilities;
			attackerIt->defense = attackerNewDefense;
			attackerIt->flagAlreadyAttacked();
		}
		if (target.hasAbility(CardAbility::DRAIN)) {
			opponentNewHealth += targetDealtDamage;
		}

		Player newMe(myNewHealth, state.me.mana, state.me.deck, state.me.runes);
		Player newOpponent(opponentNewHealth, state.opponent.mana, state.opponent.deck, state.opponent.runes);
		return GameState(newMe, newOpponent, state.opponentHand, state.cardCount, copiedCards);
	}

};

// Data model dependant utility functions

Player parse_player() {
	int health, mana, deck, runes;
	std::cin >> health >> mana >> deck >> runes;
	std::cin.ignore();
	return {
		static_cast<byte>(health),
		static_cast<ubyte>(mana),
		static_cast<ubyte>(deck),
		static_cast<ubyte>(runes) };
};

CardAbility char_to_ability(byte c) {
	switch (c) {
	case 'B':
		return CardAbility::BREAKTHROUGH;
	case 'C':
		return CardAbility::CHARGE;
	case 'G':
		return CardAbility::GUARD;
	case 'D':
		return CardAbility::DRAIN;
	case 'L':
		return CardAbility::LETHAL;
	case 'W':
		return CardAbility::WARD;
	case '-':
		return CardAbility::EMPTY;
	default:
		throw std::runtime_error("Unknown ability character '" + std::to_string(c) + "'.");
	}
};

ubyte parse_abilities(const std::string& abilities) {
	int abilityValue = 0;
	for (byte c : abilities) {
		abilityValue |= enum_value(char_to_ability(c));
	}
	return abilityValue;
};

Card parse_card() {
	int number, instanceId, location, type, cost, attack, defense;
	std::string abilities;
	int myHealthChange, opponentHealthChange, cardDraw;
	// Read data
	std::cin >>
		number >> instanceId >> location >> type >> cost >> attack >>
		defense >> abilities >> myHealthChange >> opponentHealthChange >> cardDraw;
	std::cin.ignore();
	// Create card instance
	return Card(
		number, instanceId, static_cast<CardLocation>(location), static_cast<CardType>(type), cost,
		attack, defense, parse_abilities(abilities), myHealthChange, opponentHealthChange, cardDraw,
		false, false
	);
};

GameState parse_game_state() {
	Player me = parse_player();
	Player opponent = parse_player();
	int opponentHand, cardCount;
	std::cin >> opponentHand; std::cin.ignore();
	std::cin >> cardCount; std::cin.ignore();
	std::vector<Card> cards;
	for (int i = 0; i < cardCount; ++i) {
		cards.emplace_back(parse_card());
	}
	return GameState(me, opponent, opponentHand, cardCount, std::move(cards));
};

float calculate_ability_value(CardAbility ability, const Card& card) {
	switch (ability) {
	case CardAbility::BREAKTHROUGH:
		return BREAKTHROUGH_FACTOR * card.attack;
	case CardAbility::CHARGE:
		return CHARGE_FACTOR * card.attack;
	case CardAbility::DRAIN:
		return DRAIN_FACTOR * card.attack;
	case CardAbility::GUARD:
		return GUARD_FACTOR * card.defense;
	case CardAbility::LETHAL:
		return LETHAL_FACTOR;
	case CardAbility::WARD:
		return WARD_FACTOR;
	default:
		throw std::runtime_error("Unhandled ability type.");
	}
};

float sum_ability_value(const Card& card) {
	static const std::vector<CardAbility> possibleAbilities = {
		CardAbility::BREAKTHROUGH, CardAbility::CHARGE, CardAbility::DRAIN,
		CardAbility::GUARD, CardAbility::GUARD, CardAbility::WARD
	};
	float value = 0.0f;
	for (auto ability : possibleAbilities) {
		if (card.hasAbility(ability)) {
			value += calculate_ability_value(ability, card);
		}
	}
	return value;
};

float calculate_value(const Card& card) {
	float attack = std::abs(card.attack) * ATTACK_FACTOR;
	float defense = std::abs(card.defense) * DEFENSE_FACTOR;
	float myHealth = card.myHealthChange * PLAYER_HP_FACTOR;
	float opponentHealth = card.opponentHealthChange * OPPONENT_HP_FACTOR;
	float cardDraw = card.cardDraw * CARD_DRAW_FACTOR;
	float abilities = sum_ability_value(card);
	return attack + defense + myHealth + opponentHealth + cardDraw + abilities;
}

float calculate_draft_value(const Card& card) {
	return calculate_value(card) - (card.cost * 2.0f);
};

std::vector<std::unique_ptr<Action>> collect_possible_actions(const GameState& state) {
	std::vector<std::unique_ptr<Action>> possibleActions;
	std::vector<const Card*> playableCards;
	std::vector<const Card*> cardsOnMySide;
	std::vector<const Card*> cardsOnOpponentSide;
	std::vector<const Card*> guardsOnOpponentSide;
	// Categorize cards
	for (const auto& card : state.cards) {
		switch (card.location) {
		case CardLocation::HAND:
			if (card.cost <= state.me.mana) {
				playableCards.emplace_back(&card);
			}
			break;
		case CardLocation::MY_SIDE:
			cardsOnMySide.emplace_back(&card);
			break;
		case CardLocation::OPPONENT_SIDE:
			cardsOnOpponentSide.emplace_back(&card);
			if (card.hasAbility(CardAbility::GUARD)) {
				guardsOnOpponentSide.emplace_back(&card);
			}
			break;
		}
	}

	// Add summon and use actions
	bool isBoardFull = (cardsOnMySide.size() == BOARD_SIZE);
	for (const auto card : playableCards) {
		switch (card->type) {
		case CardType::CREATURE:
			if (!isBoardFull) {
				possibleActions.emplace_back(std::make_unique<SummonAction>(*card));
			}
			break;
		case CardType::GREEN_ITEM:
			for (const auto& friendlyCreature : cardsOnMySide) {
				possibleActions.emplace_back(std::make_unique<UseGreenItemAction>(*card, *friendlyCreature));
			}
			break;
		case CardType::RED_ITEM:
			for (const auto& hostileCreature : cardsOnOpponentSide) {
				possibleActions.emplace_back(std::make_unique<UseRedItemAction>(*card, *hostileCreature));
			}
			break;
		case CardType::BLUE_ITEM:
			if (card->defense < 0) {
				// It is possible to use the card on creatures
				for (const auto& hostileCreature : cardsOnOpponentSide) {
					possibleActions.emplace_back(std::make_unique<UseBlueItemAction>(*card, new Card(*hostileCreature)));
				}
			}
			possibleActions.emplace_back(std::make_unique<UseBlueItemAction>(*card, nullptr));
		}
	}

	// Add attack actions
	for (const auto card : cardsOnMySide) {
		if (!card->canAttack()) {
			continue;
		}
		if (guardsOnOpponentSide.empty()) {
			possibleActions.emplace_back(std::make_unique<AttackFaceAction>(*card));
			for (const auto hostileCreature : cardsOnOpponentSide) {
				possibleActions.emplace_back(std::make_unique<AttackCreatureAction>(*card, *hostileCreature));
			}
		}
		else {
			for (const auto guard : guardsOnOpponentSide) {
				possibleActions.emplace_back(std::make_unique<AttackCreatureAction>(*card, *guard));
			}
		}
	}
	return possibleActions;
}

float evaluate_game_state(const GameState& state) {
	if (state.isLethal()) {
		// Never miss lethal
		return std::numeric_limits<float>::max();
	}
	if (state.hasLost()) {
		// Do not kill myself, preferably
		return std::numeric_limits<float>::lowest();
	}
	float value = 0.0f;
	value += state.me.health;
	value -= state.opponent.health;
	for (const auto& card : state.cards) {
		switch (card.location) {
		case CardLocation::MY_SIDE:
			value += calculate_value(card);
			break;
		case CardLocation::OPPONENT_SIDE:
			value -= calculate_value(card);
			break;
		}
	}
	return value;
}

struct GameStateNode {
	
	using ChildrenContainer = std::unordered_map<std::unique_ptr<Action>, std::unique_ptr<GameStateNode>>;

	float value;
	ChildrenContainer children;

	GameStateNode(float value, ChildrenContainer&& children)
		: value(value), children(std::move(children)) {}

};

std::unique_ptr<GameStateNode> calculate_tree_in_time_frame(const GameState& state, long long startMillis, long long maxMillis) {
	auto possibleActions = collect_possible_actions(state);
	if (possibleActions.empty()) {
		return std::make_unique<GameStateNode>(evaluate_game_state(state), GameStateNode::ChildrenContainer());
	}
	GameStateNode::ChildrenContainer children;
	for (auto& action : possibleActions) {
		auto millis = current_time_in_millis();
		if (millis - startMillis >= maxMillis) {
			return std::make_unique<GameStateNode>(evaluate_game_state(state), std::move(children));
		}
		GameState resultingState = action->getResult(state);
		children.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(std::move(action)),
			std::forward_as_tuple(calculate_tree_in_time_frame(resultingState, startMillis, maxMillis)));
	}
	return std::make_unique<GameStateNode>(evaluate_game_state(state), std::move(children));
}

std::pair<float, std::vector<Action*>> max_sequence(const GameStateNode& node, const std::vector<Action*>& actions) {
	std::pair<float, std::vector<Action*>> maxValueSequence(node.value, actions);
	for (const auto& entry : node.children) {
		std::vector<Action*> stepsToChildren(actions);
		stepsToChildren.emplace_back(entry.first.get());
		auto maxChildSequence = max_sequence(*entry.second, stepsToChildren);
		if (maxChildSequence.first > maxValueSequence.first) {
			maxValueSequence = maxChildSequence;
		}
	}
	return maxValueSequence;
}

void play_drafting_turn(const GameState& state) {
	auto bestCardIt = max(state.cards, [](const auto& c1, const auto& c2) {
		return calculate_draft_value(c1) < calculate_draft_value(c2);
	});
	std::cout << PickAction(bestCardIt - state.cards.begin()).getCommand() << std::endl;
}

void play_real_turn(const GameState& state) {
	GameState currentState(state);
	std::string actionBuffer;
	auto rootNode = calculate_tree_in_time_frame(state, current_time_in_millis(), TREE_BUILDING_MAX_MILLIS);
	auto maxSequence = max_sequence(*rootNode, {});
	for (auto action : maxSequence.second) {
		if (!actionBuffer.empty()) {
			actionBuffer += ";";
		}
		actionBuffer += action->getCommand();
	}

	if (actionBuffer.empty()) {
		actionBuffer = "PASS";
	}
	std::cout << actionBuffer << std::endl;
}

int main(int argc, byte** argv) {
	while (true) {
		GameState state(parse_game_state());
		if (state.isDraftingPhase()) {
			play_drafting_turn(state);
		}
		else {
			play_real_turn(state);
		}
	}
	return 0;
}
