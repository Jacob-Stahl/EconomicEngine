<script lang="ts">
  import WidgetGrid from './lib/components/WidgetGrid.svelte';
  import type { WidgetFrameModel } from './lib/layout/types';

  const gridScale = 3;
  const columns = 12 * gridScale;
  let widgetCount = 4;

  function scaleUnits(value: number) {
    return value * gridScale;
  }

  let widgets: WidgetFrameModel[] = [
    { id: 'widget-1', name: 'Spread', x: scaleUnits(0), y: scaleUnits(0), width: scaleUnits(6), height: scaleUnits(2) },
    { id: 'widget-2', name: 'Depth', x: scaleUnits(6), y: scaleUnits(0), width: scaleUnits(6), height: scaleUnits(2) },
    { id: 'widget-3', name: 'Orders', x: scaleUnits(0), y: scaleUnits(2), width: scaleUnits(4), height: scaleUnits(2) },
  ];

  function overlaps(a: WidgetFrameModel, b: WidgetFrameModel) {
    return (
      a.x < b.x + b.width &&
      a.x + a.width > b.x &&
      a.y < b.y + b.height &&
      a.y + a.height > b.y
    );
  }

  function canPlace(candidate: WidgetFrameModel) {
    return (
      candidate.x >= 0 &&
      candidate.y >= 0 &&
      candidate.x + candidate.width <= columns &&
      widgets.every((widget) => widget.id === candidate.id || !overlaps(widget, candidate))
    );
  }

  function findOpenPosition(width: number, height: number) {
    for (let y = 0; y < 100; y += 1) {
      for (let x = 0; x <= columns - width; x += 1) {
        const candidate: WidgetFrameModel = {
          id: 'candidate',
          name: 'candidate',
          x,
          y,
          width,
          height,
        };

        if (canPlace(candidate)) {
          return { x, y };
        }
      }
    }

    return {
      x: 0,
      y: widgets.reduce((maxY, widget) => Math.max(maxY, widget.y + widget.height), 0),
    };
  }

  function addWidget() {
    const width = scaleUnits(4);
    const height = scaleUnits(2);
    const position = findOpenPosition(width, height);

    widgets = [
      ...widgets,
      {
        id: `widget-${widgetCount}`,
        name: `Widget ${widgetCount}`,
        ...position,
        width,
        height,
      },
    ];

    widgetCount += 1;
  }

  function handleChange(event: CustomEvent<{ widgets: WidgetFrameModel[] }>) {
    widgets = event.detail.widgets;
  }
</script>

<div class="app-shell">
  <section class="toolbar">
    <button type="button" class="add-button" on:click={addWidget}>
      Add widget
    </button>
  </section>

  <WidgetGrid {widgets} {columns} on:change={handleChange} let:widget>
    <div class="widget-body">
      <p class="widget-label">Position {widget.x}, {widget.y}</p>
      <p class="widget-meta">Size {widget.width} x {widget.height}</p>
    </div>
  </WidgetGrid>
</div>

<style>
  .app-shell {
    display: grid;
    gap: 1rem;
  }

  .toolbar {
    display: flex;
    align-items: center;
    min-height: 2.5rem;
    padding: 0.25rem 0.5rem;
    border-bottom: 1px solid rgba(0, 0, 0, 0.35);
  }

  .widget-body {
    display: grid;
  }
</style>
