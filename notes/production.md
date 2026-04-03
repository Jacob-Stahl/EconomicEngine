# Recipes

- Recipes provide ways to transmute assets into other assets. They have inputs and outputs. Agents (like Manufacturers) follow recipes to produce assets.

```json
{
    "inputs": {"ORE": 2, "LABOR": 1, "ENERGY": 10},
    "outputs": {"METAL": 1},
    "cost": 15
},
```

- ?? They can be a be a stand in for technology. In the early stages of a civilization, only simple and inefficent recipes are feasible for Agents to exploit. As an economy develops, new recipes become feasible as feedstock drops in price, and more complex assets are produced.

- ?? How do manufacturers plan to use new recipes? It might be advantagious to follow a Recipe that turns NUCLEAR_FUEL into large amounts of ENERGY, but not if there are no agents producing NUCLEAR_FUEL. Conversely, why would an agent follow an expensive Recipe to produce NUCLEAR_FUEL if there are no buyers. This is a catch 22.

- ?? https://en.wikipedia.org/wiki/Hypergraph
- ?? A collection of recipes can be modeled as Hypergraph, where every asset is a node, and every Recipe is a hyperedge connecting multiple nodes.

- ?? A vertically integrated SuperManufacturer or some kind of central planner could observe the market state, and find the most advantagous path through the Recipe hyper graph that maximizes profit or minimizes the cost of [People's Needs](people.md).

# Production

- ?? Production could be monitored by displaying a directed graph, where each node is an asset, and each edge is the net production flow rate from one asset to the other.
