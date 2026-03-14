function drawLine(ctx, points, getX, getY, color) {
    let drawing = false;
    ctx.beginPath();

    points.forEach(point => {
        const value = point.value;
        if (value === null || value === undefined) {
            drawing = false;
            return;
        }

        const x = getX(point.index);
        const y = getY(value);

        if (!drawing) {
            ctx.moveTo(x, y);
            drawing = true;
        } else {
            ctx.lineTo(x, y);
        }
    });

    ctx.strokeStyle = color;
    ctx.lineWidth = 2;
    ctx.stroke();

    points.forEach(point => {
        const value = point.value;
        if (value === null || value === undefined) {
            return;
        }

        const x = getX(point.index);
        const y = getY(value);
        ctx.beginPath();
        ctx.arc(x, y, 3, 0, Math.PI * 2);
        ctx.fillStyle = color;
        ctx.fill();
    });
}

function drawDepthSeries(ctx, bins, getX, getY, color, fillColor) {
    if (bins.length === 0) {
        return;
    }

    ctx.beginPath();
    ctx.moveTo(getX(bins[0].price), getY(0));

    bins.forEach(bin => {
        ctx.lineTo(getX(bin.price), getY(bin.qty));
    });

    ctx.lineTo(getX(bins[bins.length - 1].price), getY(0));
    ctx.closePath();
    ctx.fillStyle = fillColor;
    ctx.fill();

    ctx.beginPath();
    bins.forEach((bin, index) => {
        const x = getX(bin.price);
        const y = getY(bin.qty);

        if (index === 0) {
            ctx.moveTo(x, y);
        } else {
            ctx.lineTo(x, y);
        }
    });
    ctx.strokeStyle = color;
    ctx.lineWidth = 2;
    ctx.stroke();

    bins.forEach(bin => {
        ctx.beginPath();
        ctx.arc(getX(bin.price), getY(bin.qty), 3, 0, Math.PI * 2);
        ctx.fillStyle = color;
        ctx.fill();
    });
}

function normalizeBins(source) {
    if (!source) {
        return [];
    }

    if (Array.isArray(source)) {
        return source.map(bin => ({
            price: bin.price,
            qty: bin.qty ?? bin.totalQty ?? 0,
        }));
    }

    if (typeof source.size === 'function' && typeof source.get === 'function') {
        const bins = [];

        for (let i = 0; i < source.size(); i++) {
            const bin = source.get(i);
            bins.push({ price: bin.price, qty: bin.qty ?? bin.totalQty ?? 0 });
        }

        return bins;
    }

    return [];
}

function setMetaText(metaElement, text) {
    if (metaElement) {
        metaElement.textContent = text;
    }
}

export function createSpreadChart({ canvas, metaElement, historyWindow = 500 }) {
    const ctx = canvas.getContext('2d');
    const history = [];

    function pushPoint(time, spread) {
        history.push({
            time,
            bid: spread && !spread.bidsMissing ? spread.highestBid : null,
            ask: spread && !spread.asksMissing ? spread.lowestAsk : null,
        });

        if (history.length > historyWindow) {
            history.shift();
        }
    }

    function render() {
        const width = canvas.width;
        const height = canvas.height;
        const padding = { top: 24, right: 16, bottom: 32, left: 42 };

        ctx.clearRect(0, 0, width, height);
        ctx.fillStyle = '#ffffff';
        ctx.fillRect(0, 0, width, height);

        if (history.length === 0) {
            ctx.fillStyle = '#666';
            ctx.font = '14px monospace';
            ctx.textAlign = 'center';
            ctx.fillText('Waiting for ticks...', width / 2, height / 2);
            setMetaText(metaElement, `Tracking latest ${historyWindow} ticks.`);
            return;
        }

        const values = history.flatMap(point => {
            const series = [];
            if (point.bid !== null) series.push(point.bid);
            if (point.ask !== null) series.push(point.ask);
            return series;
        });

        const minValue = values.length ? Math.min(...values) : 0;
        const maxValue = values.length ? Math.max(...values) : 1;
        const range = Math.max(1, maxValue - minValue);
        const chartWidth = width - padding.left - padding.right;
        const chartHeight = height - padding.top - padding.bottom;
        const lastIndex = Math.max(1, history.length - 1);

        const getX = index => padding.left + (chartWidth * index) / lastIndex;
        const getY = value => padding.top + chartHeight - ((value - minValue) / range) * chartHeight;

        ctx.strokeStyle = '#e2e2e2';
        ctx.lineWidth = 1;

        for (let i = 0; i <= 4; i++) {
            const y = padding.top + (chartHeight * i) / 4;
            ctx.beginPath();
            ctx.moveTo(padding.left, y);
            ctx.lineTo(width - padding.right, y);
            ctx.stroke();
        }

        ctx.beginPath();
        ctx.moveTo(padding.left, padding.top);
        ctx.lineTo(padding.left, height - padding.bottom);
        ctx.lineTo(width - padding.right, height - padding.bottom);
        ctx.strokeStyle = '#999';
        ctx.stroke();

        const bidPoints = history.map((point, index) => ({ index, value: point.bid }));
        const askPoints = history.map((point, index) => ({ index, value: point.ask }));

        drawLine(ctx, bidPoints, getX, getY, '#0f766e');
        drawLine(ctx, askPoints, getX, getY, '#b91c1c');

        ctx.fillStyle = '#333';
        ctx.font = '12px monospace';
        ctx.textAlign = 'left';
        ctx.fillText(`${maxValue}`, 8, padding.top + 4);
        ctx.fillText(`${minValue}`, 8, height - padding.bottom);

        const firstTime = history[0].time;
        const lastTime = history[history.length - 1].time;
        ctx.textAlign = 'left';
        ctx.fillText(`t=${firstTime}`, padding.left, height - 8);
        ctx.textAlign = 'right';
        ctx.fillText(`t=${lastTime}`, width - padding.right, height - 8);

        ctx.textAlign = 'left';
        ctx.fillStyle = '#0f766e';
        ctx.fillText('Bid', padding.left, 14);
        ctx.fillStyle = '#b91c1c';
        ctx.fillText('Ask', padding.left + 48, 14);

        const latest = history[history.length - 1];
        setMetaText(
            metaElement,
            `Tracking latest ${historyWindow} ticks. Latest tick ${latest.time}: bid=${latest.bid ?? 'NA'} ask=${latest.ask ?? 'NA'}`
        );
    }

    function reset() {
        history.length = 0;
        render();
    }

    return {
        pushPoint,
        render,
        reset,
    };
}

export function createDepthChart({ canvas, metaElement }) {
    const ctx = canvas.getContext('2d');

    function render({ depth, spread, time }) {
        const width = canvas.width;
        const height = canvas.height;
        const padding = { top: 24, right: 16, bottom: 32, left: 42 };

        ctx.clearRect(0, 0, width, height);
        ctx.fillStyle = '#ffffff';
        ctx.fillRect(0, 0, width, height);

        const bidBins = normalizeBins(depth ? depth.bidBins : null).sort((a, b) => a.price - b.price);
        const askBins = normalizeBins(depth ? depth.askBins : null).sort((a, b) => a.price - b.price);

        if (bidBins.length === 0 && askBins.length === 0) {
            ctx.fillStyle = '#666';
            ctx.font = '14px monospace';
            ctx.textAlign = 'center';
            ctx.fillText('No depth on book.', width / 2, height / 2);
            setMetaText(metaElement, `Tick ${time}: no bid or ask depth.`);
            return;
        }

        const prices = bidBins.concat(askBins).map(bin => bin.price);
        const qtys = bidBins.concat(askBins).map(bin => bin.qty);
        const minPrice = Math.min(...prices);
        const maxPrice = Math.max(...prices);
        const maxQty = Math.max(1, ...qtys);
        const chartWidth = width - padding.left - padding.right;
        const chartHeight = height - padding.top - padding.bottom;
        const priceRange = Math.max(1, maxPrice - minPrice);
        const getX = price => padding.left + ((price - minPrice) / priceRange) * chartWidth;
        const getY = qty => padding.top + chartHeight - (qty / maxQty) * chartHeight;

        ctx.strokeStyle = '#e2e2e2';
        ctx.lineWidth = 1;
        for (let i = 0; i <= 4; i++) {
            const y = padding.top + (chartHeight * i) / 4;
            ctx.beginPath();
            ctx.moveTo(padding.left, y);
            ctx.lineTo(width - padding.right, y);
            ctx.stroke();
        }

        ctx.beginPath();
        ctx.moveTo(padding.left, padding.top);
        ctx.lineTo(padding.left, height - padding.bottom);
        ctx.lineTo(width - padding.right, height - padding.bottom);
        ctx.strokeStyle = '#999';
        ctx.stroke();

        drawDepthSeries(ctx, bidBins, getX, getY, '#0f766e', 'rgba(15, 118, 110, 0.16)');
        drawDepthSeries(ctx, askBins, getX, getY, '#b91c1c', 'rgba(185, 28, 28, 0.16)');

        ctx.fillStyle = '#333';
        ctx.font = '12px monospace';
        ctx.textAlign = 'left';
        ctx.fillText(`${maxQty}`, 8, padding.top + 4);
        ctx.fillText('0', 18, height - padding.bottom);
        ctx.fillText(`${minPrice}`, padding.left, height - 8);
        ctx.textAlign = 'right';
        ctx.fillText(`${maxPrice}`, width - padding.right, height - 8);

        ctx.textAlign = 'left';
        ctx.fillStyle = '#0f766e';
        ctx.fillText('Bid Depth', padding.left, 14);
        ctx.fillStyle = '#b91c1c';
        ctx.fillText('Ask Depth', padding.left + 84, 14);

        setMetaText(
            metaElement,
            `Tick ${time}: ${bidBins.length} bid bins, ${askBins.length} ask bins, top bid=${spread && !spread.bidsMissing ? spread.highestBid : 'NA'}, top ask=${spread && !spread.asksMissing ? spread.lowestAsk : 'NA'}`
        );
    }

    function reset(time = 'NA') {
        render({ depth: null, spread: null, time });
    }

    return {
        render,
        reset,
    };
}