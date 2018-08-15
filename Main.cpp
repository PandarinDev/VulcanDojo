#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <type_traits>

// Constants

static constexpr auto MATH_PI = 3.14159265358979323846f;
static constexpr auto BOARD_SIZE = 6;
static constexpr auto HAND_SIZE = 8;
static constexpr auto CURVE_MEAN = 3.0f;
static constexpr auto CURVE_VARIANCE = 3.0f;
static constexpr auto CURVE_VALUE = 5.0f;

// Card stat factors for weighting

static constexpr auto ATTACK_FACTOR = 1.0f;
static constexpr auto DEFENSE_FACTOR = 0.8f;
static constexpr auto PLAYER_HP_FACTOR = 0.8f;
static constexpr auto OPPONENT_HP_FACTOR = 1.0f;
static constexpr auto CARD_DRAW_FACTOR = 0.5f;

// Card ability factors for weighting

static constexpr auto BREAKTHROUGH_FACTOR = 0.15f;
static constexpr auto CHARGE_FACTOR = 0.5f;
static constexpr auto DRAIN_FACTOR = 0.075f;
static constexpr auto GUARD_FACTOR = 0.2f;
static constexpr auto LETHAL_FACTOR = 2.0f;
static constexpr auto WARD_FACTOR = 1.0f;

// General utility functions

auto enum_value = [](auto e) -> auto {
	return static_cast<std::underlying_type_t<decltype(e)>>(e);
};

auto filter_collection = [](const auto& collection, auto predicate) {
	std::decay_t<decltype(collection)> filtered_collection;
	std::copy_if(collection.begin(), collection.end(), std::back_inserter(filtered_collection), predicate);
	return filtered_collection;
};

auto max = [](const auto& collection, auto predicate) {
	return std::max_element(collection.begin(), collection.end(), predicate);
};

auto min = [](const auto& collection, auto predicate) {
	return std::min_element(collection.begin(), collection.end(), predicate);
};

auto standard_gaussian = [](float x) -> float {
	return std::exp(-x * x / 2.0f) / std::sqrt(2.0f * MATH_PI);
};

auto general_gaussian = [](float x, float mu, float sigma) -> float {
	return standard_gaussian((x - mu) / sigma) / sigma;
};

// Data models

struct Player {
	int health;
	int mana;
	int deck;
	int runes;

	Player(int health, int mana, int deck, int runes)
		: health(health), mana(mana), deck(deck), runes(runes) {}
	Player(const Player&) = default;
	Player& operator=(const Player&) = default;

};

enum class CardLocation : int {
	HAND = 0,
	MY_SIDE = 1,
	OPPONENT_SIDE = -1
};

enum class CardType : int {
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
	WARD = 0b100000,
};

struct Card {

	int number;
	int instanceId;
	CardLocation location;
	CardType type;
	int cost;
	int attack;
	int defense;
	int abilities;
	int myHealthChange;
	int opponentHealthChange;
	int cardDraw;
	bool playedThisTurn;
	bool alreadyAttacked;

	Card(
		int number, int instanceId, CardLocation location,
		CardType type, int cost, int attack, int defense,
		int abilities, int myHealthChange, int opponentHealthChange,
		int cardDraw, bool playedThisTurn, bool alreadyAttacked)
		: number(number), instanceId(instanceId), location(location),
		type(type), cost(cost), attack(attack), defense(defense), abilities(abilities),
		myHealthChange(myHealthChange), opponentHealthChange(opponentHealthChange),
		cardDraw(cardDraw), playedThisTurn(playedThisTurn), alreadyAttacked(alreadyAttacked) {}

	bool canAttack() const {
		return !alreadyAttacked && (!playedThisTurn || hasAbility(CardAbility::CHARGE));
	}

	bool hasAbility(CardAbility ability) const {
		return (abilities & enum_value(ability)) != 0;
	}

	void removeAbility(CardAbility ability) {
		abilities &= ~enum_value(ability);
	}

};

struct GameState {

	Player me;
	Player opponent;
	int opponentHand;
	int cardCount;
	std::vector<std::shared_ptr<Card>> cards;

	GameState(Player me, Player opponent, int opponentHand, int cardCount, std::vector<std::shared_ptr<Card>>&& cards)
		: me(me), opponent(opponent), opponentHand(opponentHand), cardCount(cardCount), cards(std::move(cards)) {}

	bool isDraftingPhase() const {
		return me.mana == 0;
	}

	std::vector<std::shared_ptr<Card>> getPlayableCards() const {
		return filter_collection(cards, [&](const auto& c) {
			// TODO: Later we also need to give back items
			return c->cost <= me.mana && c->location == CardLocation::HAND && c->type == CardType::CREATURE;
		});
	}

	std::vector<std::shared_ptr<Card>> getPossibleAttackers() const {
		return filter_collection(cards, [](const auto& c) {
			return c->location == CardLocation::MY_SIDE && c->canAttack();
		});
	}

	std::vector<std::shared_ptr<Card>> getOpponentGuards() const {
		return filter_collection(cards, [](const auto& c) {
			return c->location == CardLocation::OPPONENT_SIDE && c->hasAbility(CardAbility::GUARD);
		});
	}

	std::vector<std::shared_ptr<Card>> getCardsOnLocation(CardLocation location) const {
		return filter_collection(cards, [=](const auto& c) { return c->location == location; });
	}

};

class Commander {

private:

	std::shared_ptr<GameState> state;
	std::string buffer;

public:

	Commander(std::unique_ptr<GameState> state) : state(std::move(state)) {}

	const GameState& getState() const {
		return *state;
	}

	void pickCard(int i) {
		doAction("PICK " + std::to_string(i));
	}

	void summonCreature(Card& card) {
		doAction("SUMMON " + std::to_string(card.instanceId));
		state->me.mana -= card.cost;
		card.location = CardLocation::MY_SIDE;
		card.playedThisTurn = true;
	}

	void attackCreature(const Card& attacker, const Card& target) {
		doAction("ATTACK " + std::to_string(attacker.instanceId) + " " + std::to_string(target.instanceId));
	}

	void attackFace(const Card& attacker) {
		doAction("ATTACK " + std::to_string(attacker.instanceId) + " -1");
	}

	void finishTurn() {
		std::cout << buffer << std::endl;
		buffer.clear();
	}

	void doAction(const std::string& action) {
		if (!buffer.empty()) {
			buffer += ";";
		}
		buffer += action;
	}

};

// Data model dependant utility functions

Player parse_player() {
	int health, mana, deck, runes;
	std::cin >> health >> mana >> deck >> runes;
	std::cin.ignore();
	return { health, mana, deck, runes };
};

CardAbility char_to_ability(char c) {
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

int parse_abilities(const std::string& abilities) {
	int abilityValue = 0;
	for (char c : abilities) {
		abilityValue |= enum_value(char_to_ability(c));
	}
	return abilityValue;
};

std::shared_ptr<Card> parse_card() {
	int number, instanceId, location, type, cost, attack, defense;
	std::string abilities;
	int myHealthChange, opponentHealthChange, cardDraw;
	// Read data
	std::cin >>
		number >> instanceId >> location >> type >> cost >> attack >>
		defense >> abilities >> myHealthChange >> opponentHealthChange >> cardDraw;
	std::cin.ignore();
	// Create card instance
	return std::make_shared<Card>(
		number, instanceId, static_cast<CardLocation>(location), static_cast<CardType>(type), cost,
		attack, defense, parse_abilities(abilities), myHealthChange, opponentHealthChange, cardDraw,
		false, false
	);
};

std::unique_ptr<GameState> parse_game_state() {
	Player me = parse_player();
	Player opponent = parse_player();
	int opponentHand, cardCount;
	std::cin >> opponentHand; std::cin.ignore();
	std::cin >> cardCount; std::cin.ignore();
	std::vector<std::shared_ptr<Card>> cards;
	for (int i = 0; i < cardCount; ++i) {
		cards.emplace_back(parse_card());
	}
	return std::make_unique<GameState>(me, opponent, opponentHand, cardCount, std::move(cards));
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

float calculate_curve_value(const Card& card) {
	return general_gaussian(card.cost, CURVE_MEAN, CURVE_VARIANCE) * CURVE_VALUE;
};

float sum_ability_value(const Card& card) {
	std::vector<CardAbility> possibleAbilities = {
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

float calculate_draft_value(const Card& card) {
	float attack = std::abs(card.attack) * ATTACK_FACTOR;
	float defense = std::abs(card.defense) * DEFENSE_FACTOR;
	float myHealth = card.myHealthChange * PLAYER_HP_FACTOR;
	float opponentHealth = card.opponentHealthChange * OPPONENT_HP_FACTOR;
	float cardDraw = card.cardDraw * CARD_DRAW_FACTOR;
	float abilities = sum_ability_value(card);
	float value = attack + defense + myHealth + opponentHealth + cardDraw + abilities;
	value *= calculate_curve_value(card);
	return value;
};

void play_drafting_turn(Commander& commander) {
	auto cardsInHand = commander.getState().getCardsOnLocation(CardLocation::HAND);
	auto bestCardIt = max(cardsInHand, [](const auto& c1, const auto& c2) {
		return calculate_draft_value(*c1) < calculate_draft_value(*c2);
	});
	commander.pickCard(bestCardIt - cardsInHand.begin());
	commander.finishTurn();
}

void play_real_turn(Commander& commander) {
	const GameState& state = commander.getState();
	int mana = state.me.mana;
	auto playableCards = state.getPlayableCards();
	while (!playableCards.empty()) {
		auto bestCardIt = max(playableCards, [](const auto& c1, const auto& c2) {
			return calculate_draft_value(*c1) < calculate_draft_value(*c2);
		});
		commander.summonCreature(**bestCardIt);
		playableCards = state.getPlayableCards();
	}

	auto attackerCards = state.getPossibleAttackers();
	auto opponentGuards = state.getOpponentGuards();
	for (const auto& attacker : attackerCards) {
		if (!opponentGuards.empty()) {
			auto minHpGuard = min(opponentGuards, [](const auto& c1, const auto& c2) {
				return c1->defense < c2->defense;
			});
			if ((*minHpGuard)->hasAbility(CardAbility::WARD)) {
				(*minHpGuard)->removeAbility(CardAbility::WARD);
			}
			else if (attacker->attack >= (*minHpGuard)->defense) {
				opponentGuards.erase(minHpGuard);
			}
			commander.attackCreature(*attacker, **minHpGuard);
		}
		else {
			commander.attackFace(*attacker);
		}
	}
	commander.finishTurn();
}

int main(int argc, char** argv) {
	while (true) {
		Commander commander(parse_game_state());
		if (commander.getState().isDraftingPhase()) {
			play_drafting_turn(commander);
		}
		else {
			play_real_turn(commander);
		}
	}
	return 0;
}
