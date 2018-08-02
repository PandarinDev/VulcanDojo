import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashSet;
import java.util.List;
import java.util.Scanner;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * CodinGame requires the class to be named 'Player' and to be package-private.
 */
class Player {
    
    // Constants
    private static final int BOARD_SIZE = 6;
    // Stat factors
    private static final float ATTACK_FACTOR = 1.0f;
    private static final float DEFENSE_FACTOR = 0.8f;
    private static final float PLAYER_HP_FACTOR = 0.8f;
    private static final float ENEMY_HP_FACTOR = 1.0f;
    private static final float CARD_DRAW_FACTOR = 0.5f;
    // Ability factors
    private static final float BREAKTHROUGH_FACTOR = 0.15f;
    private static final float CHARGE_FACTOR = 0.5f;
    private static final float DRAIN_FACTOR = 0.075f;
    private static final float GUARD_FACTOR = 0.2f;
    private static final float LETHAL_FACTOR = 2.0f;
    private static final float WARD_FACTOR = 1.0f;
    // Curve factors
    private static final float CURVE_MEAN = 3.0f;
    private static final float CURVE_VARIANCE = 3.0f;
    private static final float CURVE_VALUE = 5.0f;
    
    public static void main(String[] args) {
        new Player().start();
    }
    
    public void start() {
        Scanner in = new Scanner(System.in);
        DraftingLogic draftingLogic = new DraftingLogic();
        while (true) {
            // Parse the gamblers
            Gambler[] players = new Gambler[] {
                parseGambler(in), parseGambler(in)
            };
            Gambler me = players[0];
            Gambler opponent = players[1];
            // Parse the game state
            GameState gameState = parseGameState(in);
            // Use different logic for drafting instead of...
            if (isDraftingPhase(me)) {
                draftingLogic.playTurn(me, opponent, gameState);
            }
            // Playing the game regularly
            else {
                new SimplePlayLogic().playTurn(me, opponent, gameState);
            }
        }
    }
    
    // Class definitions
    
    public class ActionIssuer {
        
        private final StringBuffer buffer = new StringBuffer();
        private boolean empty = true;
        
        public void pick(int num) {
            doAction("PICK " + num);
        }
        
        public void summonCreature(Card card, GameState state) {
            if (card.getType() != CardType.CREATURE) {
                throw new IllegalArgumentException("Cannot summon non-creature card.");
            }
            if (state.isMyBoardFull()) {
                throw new IllegalStateException("Cannot summon creature, my board is already full.");
            }
            card.setLocation(CardLocation.MY_SIDE);
            card.flagSummonedThisTurn();
            doAction("SUMMON " + card.getInstanceId());
        }
        
        public void attackCreature(Card attacker, Card target) {
            if (attacker.getType() != CardType.CREATURE) {
                throw new IllegalArgumentException("Cannot attack with non-creature card.");
            }
            if (target.getType() != CardType.CREATURE) {
                throw new IllegalArgumentException("Cannot attack non-creature card.");
            }
            if (!attacker.canAttack()) {
                throw new IllegalArgumentException("This creature cannot attack now.");
            }
            attacker.flagAttackedThisTurn();
            doAction("ATTACK " + attacker.getInstanceId() + " " + target.getInstanceId());
        }
        
        public void attackFace(Card attacker) {
            if (attacker.getType() != CardType.CREATURE) {
                throw new IllegalArgumentException("Cannot attack with non-creature card.");
            }
            doAction("ATTACK " + attacker.getInstanceId() + " -1");
        }
        
        public void useItemOnCreature(Card item, Card creature) {
            if (item.getType() == CardType.CREATURE) {
                throw new IllegalArgumentException("Cannot use creature as an item.");
            }
            if (creature.getType() != CardType.CREATURE) {
                throw new IllegalArgumentException("Cannot target non-creature card with item.");
            }
            doAction("USE " + item.getInstanceId() + " " + creature.getInstanceId());
        }
        
        public void useBlueItem(Card item) {
            if (item.getType() != CardType.BLUE_ITEM) {
                throw new IllegalArgumentException("Only blue items can be used on players.");
            }
            doAction("USE " + item.getInstanceId() + " -1");
        }
        
        public void pass() {
            doAction("PASS");
        }
        
        public void doAction(String action) {
            if (!empty) {
                buffer.append(";");
            }
            buffer.append(action);
            empty = false;
        }
        
        public void executeActions() {
            System.out.println(buffer.toString());
        }
        
    }
    
    /** Because the main class is named Player we must name this class something creative. */
    public class Gambler {
        
        private final int health;
        private final int mana;
        private final int cardsInDeck;
        private final int rune;

        public Gambler(int health, int mana, int cardsInDeck, int rune) {
            this.health = health;
            this.mana = mana;
            this.cardsInDeck = cardsInDeck;
            this.rune = rune;
        }

        public int getHealth() {
            return health;
        }

        public int getMana() {
            return mana;
        }

        public int getCardsInDeck() {
            return cardsInDeck;
        }

        public int getRune() {
            return rune;
        }
        
    }
    
    public enum CardLocation {
        HAND, MY_SIDE, OPPONENTS_SIDE
    }
    
    public enum CardType {
        CREATURE, GREEN_ITEM, RED_ITEM, BLUE_ITEM
    }
    
    public enum CardAbility {
        BREAKTHROUGH, CHARGE, GUARD, DRAIN, LETHAL, WARD
    }
    
    public class Card {
        
        private final int number;
        private final int instanceId;
        private CardLocation location;
        private final CardType type;
        private final int cost;
        private final int attack;
        private final int defense;
        private final Set<CardAbility> abilities;
        private final int myHealthChange;
        private final int opponentHealthChange;
        private final int cardDraw;
        private boolean attackedThisTurn = false;
        private boolean summonedThisTurn = false;
        
        public Card(
                int number, int instanceId, CardLocation location, CardType type,
                int cost, int attack, int defense, Set<CardAbility> abilities,
                int healthChange, int opponentHealthChange, int cardDraw) {
            this.number = number;
            this.instanceId = instanceId;
            this.location = location;
            this.type = type;
            this.cost = cost;
            this.attack = attack;
            this.defense = defense;
            this.abilities = abilities;
            this.myHealthChange = healthChange;
            this.opponentHealthChange = opponentHealthChange;
            this.cardDraw = cardDraw;
        }
        
        public void flagAttackedThisTurn() {
            attackedThisTurn = true;
        }
        
        public void flagSummonedThisTurn() {
            summonedThisTurn = true;
        }
        
        public boolean canAttack() {
            return
                    location == CardLocation.MY_SIDE &&
                    !hasAttackedThisTurn() &&
                    (!wasSummonedThisTurn() || abilities.contains(CardAbility.CHARGE));
        }
        
        public void setLocation(CardLocation location) {
            this.location = location;
        }
        
        public int getNumber() {
            return number;
        }

        public int getInstanceId() {
            return instanceId;
        }

        public CardLocation getLocation() {
            return location;
        }

        public CardType getType() {
            return type;
        }

        public int getCost() {
            return cost;
        }

        public int getAttack() {
            return attack;
        }

        public int getDefense() {
            return defense;
        }

        public Set<CardAbility> getAbilities() {
            return abilities;
        }

        public int getMyHealthChange() {
            return myHealthChange;
        }

        public int getOpponentHealthChange() {
            return opponentHealthChange;
        }

        public int getCardDraw() {
            return cardDraw;
        }
        
        public boolean hasAttackedThisTurn() {
            return attackedThisTurn;
        }
        
        public boolean wasSummonedThisTurn() {
            return summonedThisTurn;
        }
        
    }
    
    public class GameState {
        
        private final int numberOfCardsInOpponentsHand;
        private final List<Card> cards;

        public GameState(int numberOfCardsInOpponentsHand, List<Card> cards) {
            this.numberOfCardsInOpponentsHand = numberOfCardsInOpponentsHand;
            this.cards = cards;
        }
        
        public int getNumberOfCardsInOpponentsHand() {
            return numberOfCardsInOpponentsHand;
        }

        public List<Card> getCards() {
            return cards;
        }
        
        public List<Card> getCardsInMyHand() {
            return getCardsOnLocation(CardLocation.HAND);
        }
        
        public List<Card> getCardsOnMySide() {
            return getCardsOnLocation(CardLocation.MY_SIDE);
        }
        
        public List<Card> getCardsOnOpponentsSide() {
            return getCardsOnLocation(CardLocation.OPPONENTS_SIDE);
        }
        
        public boolean isMyBoardFull() {
            return getCardsOnMySide().size() >= BOARD_SIZE;
        }
        
        private List<Card> getCardsOnLocation(CardLocation location) {
            return cards
                    .stream()
                    .filter((card) -> card.getLocation() == location)
                    .collect(Collectors.toList());
        }
        
    }
    
    public interface GameplayLogic {
        public void playTurn(Gambler me, Gambler opponent, GameState gameState);
    }
    
    public class DraftCardComparator implements Comparator<Card> {
        
        @Override
        public int compare(Card first, Card second) {
            float firstValue = calculateDraftValue(first);
            float secondValue = calculateDraftValue(second);
            return Float.compare(firstValue, secondValue);
        }
        
        private float calculateDraftValue(Card card) {
            float attack = Math.abs(card.getAttack()) * ATTACK_FACTOR;
            float defense = Math.abs(card.getDefense()) * DEFENSE_FACTOR;
            float myHealth = card.getMyHealthChange() * PLAYER_HP_FACTOR;
            float opponentHealth = card.getOpponentHealthChange() * ENEMY_HP_FACTOR;
            float cardDraw = card.getCardDraw() * CARD_DRAW_FACTOR;
            float value = attack + defense + myHealth - opponentHealth + cardDraw;
            for (CardAbility ability : card.getAbilities()) {
                value += calculateAbilityValue(ability, card.getAttack(), card.getDefense());
            }
            value *= calculateCurveValue(card);
            return value;
        }
        
    }
    
    // Gameplay logic implementations
    
    public class DraftingLogic implements GameplayLogic {
        
        private final DraftCardComparator comparator = new DraftCardComparator();
        
        @Override
        public void playTurn(Gambler me, Gambler opponent, GameState gameState) {
            List<Card> cards = gameState.getCards();
            Card selectedCard = cards.stream().max(comparator).get();
            ActionIssuer issuer = new ActionIssuer();
            issuer.pick(cards.indexOf(selectedCard));
            issuer.executeActions();
        }
        
    }
    
    /**
     * Currently this is a simple aggressive game logic that is used
     * before decision trees are implemented. Logic is simply:
     * 1. Play highest draft value creatures from hand.
     * 2. a) If we can attack face: Attack face.
     * 2. b) Otherwise: Attack guard creature with lowest HP.
     * 
     * This logic is currently incapable of using items.
     */
    public class SimplePlayLogic implements GameplayLogic {
        
        private final DraftCardComparator comparator = new DraftCardComparator();
        
        @Override
        public void playTurn(Gambler me, Gambler opponent, GameState gameState) {
            ActionIssuer issuer = new ActionIssuer();
            // Phase#1: Play cards with the highest attack value
            int mana = me.getMana();
            List<Card> cardsInHand = gameState.getCardsInMyHand();
            List<Card> playableCards = getCreatures(getPlayableCards(mana, cardsInHand));
            while (!gameState.isMyBoardFull() && !playableCards.isEmpty()) {
                Card bestCard = getHighestDraftValueCard(playableCards);
                issuer.summonCreature(bestCard, gameState);
                cardsInHand.remove(bestCard);
                mana -= bestCard.getCost();
                playableCards = getCreatures(getPlayableCards(mana, cardsInHand));
            }
            // Phase#2: Attack face if we can, otherwise attack guard creature
            List<Card> attackers = getCreaturesThatCanAttack(gameState.getCardsOnMySide());
            for (Card attacker : attackers) {
                List<Card> enemyGuards = getGuardCreatures(gameState.getCardsOnOpponentsSide());
                if (!enemyGuards.isEmpty()) {
                    Card leastHpGuard = enemyGuards
                            .stream()
                            .min((c1, c2) -> Integer.compare(c1.getDefense(), c2.getDefense()))
                            .get();
                    issuer.attackCreature(attacker, leastHpGuard);
                }
                else {
                    issuer.attackFace(attacker);
                }
            }
            issuer.executeActions();
        }
        
        private List<Card> getPlayableCards(int mana, List<Card> cardsInHand) {
            return cardsInHand
                    .stream().
                    filter((c) -> c.getCost() <= mana)
                    .collect(Collectors.toList());
        }
        
        private List<Card> getCreatures(List<Card> cards) {
            return cards
                    .stream()
                    .filter((c) -> c.getType() == CardType.CREATURE)
                    .collect(Collectors.toList());
        }
        
        private List<Card> getCreaturesThatCanAttack(List<Card> cards) {
            return cards
                    .stream()
                    .filter((c) -> c.canAttack())
                    .collect(Collectors.toList());
        }
        
        private List<Card> getGuardCreatures(List<Card> cards) {
            return cards
                    .stream()
                    .filter((c) -> c.getAbilities().contains(CardAbility.GUARD))
                    .collect(Collectors.toList());
        }
        
        private Card getHighestDraftValueCard(List<Card> cards) {
            return cards.stream().max(comparator).get();
        }
        
    }
    
    // Parsing functions
    
    public Gambler parseGambler(Scanner in) {
        int health = in.nextInt();
        int mana = in.nextInt();
        int cardsInDeck = in.nextInt();
        int rune = in.nextInt();
        return new Gambler(health, mana, cardsInDeck, rune);
    }
    
    public GameState parseGameState(Scanner in) {
        int cardsInOpponentsHand = in.nextInt();
        int totalCards = in.nextInt();
        List<Card> cards = new ArrayList<>(totalCards);
        for (int i = 0; i < totalCards; ++i) {
            int number = in.nextInt();
            int instanceId = in.nextInt();
            CardLocation location = parseCardLocation(in.nextInt());
            CardType type = parseCardType(in.nextInt());
            int cost = in.nextInt();
            int attack = in.nextInt();
            int defense = in.nextInt();
            Set<CardAbility> abilities = parseCardAbilities(in.next());
            int healthChange = in.nextInt();
            int opponentHealthChange = in.nextInt();
            int cardDraw = in.nextInt();
            cards.add(new Card(
                    number, instanceId, location, type, cost, attack, defense,
                    abilities, healthChange, opponentHealthChange, cardDraw));
        }
        return new GameState(cardsInOpponentsHand, cards);
    }
    
    public CardLocation parseCardLocation(int location) {
        switch (location) {
            case 0:
                return CardLocation.HAND;
            case 1:
                return CardLocation.MY_SIDE;
            case -1:
                return CardLocation.OPPONENTS_SIDE;
            default:
                throw new IllegalArgumentException("Invalid location value.");
        }
    }
    
    public CardType parseCardType(int type) {
        switch (type) {
            case 0:
                return CardType.CREATURE;
            case 1:
                return CardType.GREEN_ITEM;
            case 2:
                return CardType.RED_ITEM;
            case 3:
                return CardType.BLUE_ITEM;
            default:
                throw new IllegalArgumentException("Invalid type value.");
        }
    }
    
    private Set<CardAbility> parseCardAbilities(String abilitiesStr) {
        Set<CardAbility> abilities = new HashSet<>();
        if (abilitiesStr.contains("B")) abilities.add(CardAbility.BREAKTHROUGH);
        if (abilitiesStr.contains("C")) abilities.add(CardAbility.CHARGE);
        if (abilitiesStr.contains("G")) abilities.add(CardAbility.GUARD);
        if (abilitiesStr.contains("D")) abilities.add(CardAbility.DRAIN);
        if (abilitiesStr.contains("L")) abilities.add(CardAbility.LETHAL);
        if (abilitiesStr.contains("W")) abilities.add(CardAbility.WARD);
        return abilities;
    }
    
    // Utility functions
    
    public boolean isDraftingPhase(Gambler me) {
        return me.getMana() == 0;
    }
    
    /**
     * Calculates the value/usefulness of an ability on a creature by taking it's stats into account.
     * The weights in this function are a subject to change, and it could be a good idea to experiment
     * with different values later on, or to refine the formula by taking even more factors into account.
     * @param ability The ability in question.
     * @param attack The attack of the creature.
     * @param defense The defense of the creature.
     * @return The value of the ability on the creature.
     */
    public float calculateAbilityValue(CardAbility ability, int attack, int defense) {
        switch (ability) {
            case BREAKTHROUGH:
                return BREAKTHROUGH_FACTOR * attack;
            case CHARGE:
                return CHARGE_FACTOR * attack;
            case DRAIN:
                return DRAIN_FACTOR * attack;
            case GUARD:
                return GUARD_FACTOR * defense;
            case LETHAL:
                return LETHAL_FACTOR;
            case WARD:
                return WARD_FACTOR;
            default:
                throw new IllegalArgumentException("Ability is not handled by method.");
        }
    }
    
    /**
     * Calculates how much the given card compliments our curve.
     * The target curve is subject to change and it could be a
     * good idea to experiment with different values later on.
     * @param card The card that you want to evaluate.
     * @return How much the card compliments the curve.
     */
    public float calculateCurveValue(Card card) {
        return (float) generalGaussian(
                card.getCost(), CURVE_MEAN, CURVE_VARIANCE) * CURVE_VALUE;
    }
    
    /**
     * Implements the standard Gaussian distribution.
     * @param x The input to the standard Gaussian function.
     * @return The value of the standard Gaussian function at x.
     */
    public static double standardGaussian(double x) {
        return Math.exp(-x*x / 2) / Math.sqrt(2 * Math.PI);
    }
    
    /**
     * Implements the general Gaussian distribution with mean and variance.
     * @param x The input to the standard Gaussian distribution.
     * @param mu The mean of the distribution.
     * @param sigma The variance of the distribution.
     * @return The value of the general Gaussian function at x with respect to mu and sigma.
     */
    public static double generalGaussian(double x, double mu, double sigma) {
        return standardGaussian((x - mu) / sigma) / sigma;
    }
    
}
