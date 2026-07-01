# Graph Report - .  (2026-06-30)

## Corpus Check
- cluster-only mode — file stats not available

## Summary
- 56 nodes · 79 edges · 15 communities (13 shown, 2 thin omitted)
- Extraction: 100% EXTRACTED · 0% INFERRED · 0% AMBIGUOUS
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `44d6e29a`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- [[_COMMUNITY_Community 0|Community 0]]
- [[_COMMUNITY_Community 1|Community 1]]
- [[_COMMUNITY_Community 2|Community 2]]
- [[_COMMUNITY_Community 3|Community 3]]
- [[_COMMUNITY_Community 4|Community 4]]
- [[_COMMUNITY_Community 5|Community 5]]

## God Nodes (most connected - your core abstractions)
1. `HandleEvent()` - 11 edges
2. `Update()` - 8 edges
3. `Draw()` - 7 edges
4. `Pop()` - 5 edges
5. `IsEmpty()` - 5 edges
6. `Run()` - 4 edges
7. `Begin()` - 4 edges
8. `ShowNextLine()` - 4 edges
9. `IsTyping()` - 4 edges
10. `Data()` - 3 edges

## Surprising Connections (you probably didn't know these)
- `Pop()` --calls--> `IsEmpty()`  [EXTRACTED]
  Core/Vector.h → Scene/SceneManager.cpp
- `HandleEvent()` --calls--> `Pop()`  [EXTRACTED]
  Ui/DialogueBox.cpp → Core/Vector.h
- `Run()` --calls--> `HandleEvent()`  [EXTRACTED]
  Core/Application.cpp → Ui/DialogueBox.cpp
- `Run()` --calls--> `Update()`  [EXTRACTED]
  Core/Application.cpp → Ui/DialogueBox.cpp
- `Run()` --calls--> `Draw()`  [EXTRACTED]
  Core/Application.cpp → Ui/DialogueBox.cpp

## Communities (15 total, 2 thin omitted)

### Community 0 - "Community 0"
Cohesion: 0.33
Nodes (9): Run(), EndScenario(), ShowNextLine(), IsEmpty(), OnEnter(), OnExit(), Draw(), HandleEvent() (+1 more)

### Community 1 - "Community 1"
Cohesion: 0.31
Nodes (5): Begin(), Contains(), Data(), End(), RemoveAt()

### Community 2 - "Community 2"
Cohesion: 0.32
Nodes (5): FinishTyping(), IsFinished(), IsTyping(), SetLine(), StartTyping()

### Community 3 - "Community 3"
Cohesion: 0.47
Nodes (3): Pop(), Push(), Switch()

## Knowledge Gaps
- **2 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `HandleEvent()` connect `Community 0` to `Community 2`, `Community 3`?**
  _High betweenness centrality (0.216) - this node is a cross-community bridge._
- **Why does `Pop()` connect `Community 3` to `Community 0`, `Community 1`?**
  _High betweenness centrality (0.193) - this node is a cross-community bridge._
- **Why does `Update()` connect `Community 0` to `Community 2`, `Community 3`?**
  _High betweenness centrality (0.044) - this node is a cross-community bridge._