# Hunger

- [Maslow's Heirarchy of Needs](https://en.wikipedia.org/wiki/Maslow%27s_hierarchy_of_needs#/media/File:Maslow's_Hierarchy_of_Needs_Pyramid_(original_five-level_model).png)

- Basic needs have steep hunger curves. Agents die if the go a certain amount of ticks without consuming all of these. The agent's objective should be increase it's time horizon as much as possible.
    - WATER
    - PROTEIN
    - CARBS
    - SUGAR
    - MIRCONUTRIENTS
    - CLOTHS
    - SHELTER

- ?? I suppose "assets" higher up on the pyramid could have flatter hunger curves. Do agents die without esteem? can ESTEEM be a modeled as a commodity in a way that makes sense?

- ?? In the real world, all people have the same basic biological requirements that are more easily modeled. Agents might have different preferences for wants/needs higher up on the pyramid. Higher needs (desires?) might be modeled with differently shaped hunger curves that vary between individual agents.

# Labor
- All agents produce a unit of LABOR every tick. They sell LABOR at the market rate. All manufacturer recipes should have LABOR as an ingredient. 

- ?? Using a single asset LABOR might be an over simplification. In the real world, plowing a field requires a different kind of labor than operating a nuclear power plant. It would be more realistic to have seperate categories like SKILLED_LABOR or UNSKILLED_LABOR, and let the market assign seperate values for them. LABOR could be divided into more granular specialization.

- ?? Perhaps agents could have specific specialization decided at birth. At birth, that specialization could be decided based on the instananious market value. Specializations with higher market value would tend to be filled by more agents. Agents with lower value specializations would tend to die off.

- ?? Basic needs and LABOR production change over a person's life span. Infants and children don't produce useful LABOR, and require LABOR from others until they get older. LABOR production peaks at the middle of a person's life, and declines as they get older. They may require LABOR for survival until they get older.

# Natural Lifespan
- In the real world, people have a lifespan that caps out at about 100 years. Person Agents live for a limited number of ticks before they die of "natural causes".

- ?? https://en.wikipedia.org/wiki/Bathtub_curve
- ?? Like hard-drives, human lifespans follow a bathtub distribution. They have a high failure rate when they are young that decays quickly. Failure rate slowly increases again with age.

- ?? https://en.wikipedia.org/wiki/Population_pyramid
- ?? Agent ages could be visualized and tracked with a population pyramid.

# Reproduction
- The Agent population is centrally managed by an AgentManager.
- ?? How?