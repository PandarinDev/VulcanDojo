import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashSet;
import java.util.List;
import java.util.Scanner;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * CodinGame requires the class to be named 'Player' and to be package-private.
 * This implementation is for contending in Wood 3 League. The algorithm is not
 * prepared for the advanced gameplay elements in above leagues.
 */
class Player {
    
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
        
        public void summonCreature(Card card) {
            if (card.getType() != CardType.CREATURE) {
                throw new IllegalArgumentException("Cannot summon non-creature card.");
            }
            doAction("SUMMON " + card.getInstanceId());
        }
        
        public void attackCreature(Card attacker, Card target) {
            if (attacker.getType() != CardType.CREATURE) {
                throw new IllegalArgumentException("Cannot attack with non-creature card.");
            }
            if (target.getType() != CardType.CREATURE) {
                throw new IllegalArgumentException("Cannot attack non-creature card.");
            }
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
        private final CardLocation location;
        private final CardType type;
        private final int cost;
        private final int attack;
        private final int defense;
        private final Set<CardAbility> abilities;
        private final int myHealthChange;
        private final int opponentHealthChange;
        private final int cardDraw;

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
        
        private final List<Card> selectedCards;
        
        public DraftCardComparator(List<Card> selectedCards) {
            this.selectedCards = selectedCards;
        }
        
        @Override
        public int compare(Card first, Card second) {
            float firstValue = calculateDraftValue(first);
            float secondValue = calculateDraftValue(second);
            return Float.compare(firstValue, secondValue);
        }
        
        private float calculateDraftValue(Card card) {
            int attack = card.getAttack();
            int defense = card.getDefense();
            float value = attack + defense;
            for (CardAbility ability : card.getAbilities()) {
                value += calculateAbilityValue(ability, attack, defense);
            }
            value += calculateCurveValue(card, selectedCards);
            return value;
        }
        
    }
    
    // Gameplay logic implementations
    
    public class DraftingLogic implements GameplayLogic {
        
        private final List<Card> selectedCards = new ArrayList<>();
        private final DraftCardComparator comparator = new DraftCardComparator(
                selectedCards);
        
        @Override
        public void playTurn(Gambler me, Gambler opponent, GameState gameState) {
            List<Card> cards = gameState.getCards();
            Card selectedCard = cards.stream().max(comparator).get();
            ActionIssuer issuer = new ActionIssuer();
            issuer.pick(cards.indexOf(selectedCard));
            selectedCards.add(selectedCard);
            issuer.executeActions();
        }
        
    }
    
    public class SimplePlayLogic implements GameplayLogic {

        @Override
        public void playTurn(Gambler me, Gambler opponent, GameState gameState) {
            ActionIssuer issuer = new ActionIssuer();
            // Currently we just pass until we die
            issuer.pass();
            issuer.executeActions();
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
                return 0.3f * attack;
            case CHARGE:
                return 1.0f * attack;
            case DRAIN:
                return 0.15f * attack;
            case GUARD:
                return 0.8f * defense;
            case LETHAL:
                return 3.0f;
            case WARD:
                return 1.0f / defense;
            default:
                throw new IllegalArgumentException("Ability is not handled by method.");
        }
    }
    
    /**
     * Calculates how much the given card compliments our curve.
     * The target curve is subject to change and it could be a
     * good idea to experiment with different values later on.
     * @param card The card that you want to evaluate.
     * @param selectedCards The cards that you have drafted so far.
     * @return How much the card compliments the curve.
     */
    public float calculateCurveValue(Card card, List<Card> selectedCards) {
        final float targetCurve = 3.0f;
        int totalCost = selectedCards
                .stream()
                .mapToInt((c) -> c.getCost())
                .sum();
        float currentCurve = ((float) totalCost) / selectedCards.size();
        return (1.0f / Math.abs(targetCurve - currentCurve)) * 4.0f;
    }
    
}
