const eMountainD3 = document.getElementById('mountain-d3');
const draw = SVG()
    .attr('class', 'is-fullwidth is-fullheight')
    .addTo('#canvas');
const SVGmountain = d3.create("svg")
    .attr('class', 'is-fullwidth is-fullheight')
    .attr("viewBox", [0, 0, 928, 600]);

async function readErrData(problem_id) {
    try {
        return await fetch(`../output/problem-${problem_id}.json.err`).then(e => e.text());
    } catch (e) {
        console.error(e);
        return null;
    }
}

async function _submit({problem_id}) {
    draw.clear();

    if(!(problem_id > 0)) {
        throw 'invalid problem id';
    }
    const inputData = await fetch(`../input/problem-${problem_id}.json`).then(e => e.text());
    const input = JSON.parse(inputData);

    const outputData = await fetch(`../output/problem-${problem_id}.json`)
        .then(r => {
            if(!r.ok) throw 'err';
            return r.text();
        })
        .catch(_ => '{"placements":[], "volumes":[]}');
    const output = JSON.parse(outputData);

    const dbgData = await readErrData(problem_id);
    let _musicianScores = dbgData === null ? dbgData : dbgData.split("\n").map(line => {
        const match = line.match(/^score (\d+): (\d+)$/);
        if (!match) return false;
        const [_, musician_id, score] = [...match].map(e => parseInt(e));
        return {musician_id, score};
    })
    .filter(e => e);
    const musicianScores = (_musicianScores === null || _musicianScores.length === 0) ? null : _musicianScores;

    const g = draw.group();

    const room = {
        width: input['room_width'],
        height: input['room_height'],
    };
    draw.attr('viewBox', `0 0 ${room.width} ${room.height}`)

    const stage = {
        width: input['stage_width'],
        height: input['stage_height'],
        x: input['stage_bottom_left'][0],
        y: input['stage_bottom_left'][1],
    };

    // room
    g.rect(room.width, room.height)
        .fill('#fff')
        .stroke('black');

    // stage
    g.rect(stage.width, stage.height)
        .move(stage.x, stage.y)
        .fill('#ddd');

    // pillars
    const pillars = input['pillars'];
    for(const pillar of pillars) {
        const {center: [x, y], radius} = pillar;
        g.circle({cx: x, cy: y})
            .radius(radius).fill('#f00');
    }

    // attendees
    const attendees = input['attendees'];
    for(const attendee of attendees) {
        const {x, y, _tastes} = attendee;
        g.circle({cx: x, cy: y})
            .radius(5).fill('#0f0');
    }

    // musicians
    //  - volume represented as transparecy
    //  - score reperesented as color
    const placements = output['placements'];
    const volumes = output['volumes'];
    const maxScore = musicianScores === null ? 0 : musicianScores.reduce((acc, cur) => Math.max(acc, cur.score), 0);
    const minScore = musicianScores === null ? 0 : musicianScores.reduce((acc, cur) => Math.min(acc, cur.score), 0);
    for (let i = 0; i < placements.length; i++) {
        const {x, y} = placements[i];
        const volume = volumes[i];

        const {red,green,blue} = musicianScores === null ? {red: 0, green: 0, blue: 255} : (() => {
            const score = musicianScores[i].score;
            const scoreRatio = (score - minScore) / (maxScore - minScore);
            const red = Math.floor(255 * scoreRatio);
            const green = 0;
            const blue = Math.floor(255 * (1 - scoreRatio));;
            return {red, green, blue};
        })();

        const circle = g.circle({cx: x, cy: y})
            .radius(10)
            .fill(`rgba(${red}, ${green}, ${blue}, ${(1+volume)/20})`)
            .on('mouseover', () => {
                const tooltip = document.createElement('div');
                const circlePosition = circle.node.getBoundingClientRect();
                tooltip.classList.add('tooltip');
                tooltip.style.position = 'fixed';
                tooltip.style.top = `${circlePosition.top + 5}px`;
                tooltip.style.left = `${circlePosition.left + 5}px`;
                tooltip.innerHTML = `
                    <div class="card">
                        <div class="card-content">
                            <div class="content">
                                <p>ID: ${i}</p>
                                <p>Score: ${musicianScores === null ? 'N/A' : musicianScores[i].score}</p>
                                <p>Volume: ${volume}</p>
                                <p>Instrument: ${input['musicians'][i]}</p>
                            </div>
                        </div>
                    </div>
                `;
                document.body.appendChild(tooltip);
            })
            .on('mouseout', () => {
                const tooltip = document.querySelector('.tooltip');
                if(tooltip) {
                    tooltip.remove();
                }
            });
    }

    // Handle zoom
    const handleZoom = (e) => {
        d3.select("svg g").attr("transform", e.transform);
    };
    const zoom = d3.zoom()
        .on("zoom", handleZoom);
    d3.select("svg").call(zoom);
}

async function drawMountain({problem_id}) {
    SVGmountain.selectAll("*").remove();
    /***
     * Base: Line Chart https://observablehq.com/@d3/line-chart/2?intent=fork
     * Bar Chart https://observablehq.com/@d3/bar-chart/2?intent=fork
     * https://d3js.org/getting-started#d3-in-vanilla-html
     *
     */

    const dataRaw = await fetch(`../output/problem-${problem_id}.json.err`)
        .then(r => {
            if(!r.ok) throw 'err';
            return r.text();
        })
        .catch(e => {
            console.log(e);
            return '';
        });
    const data = dataRaw
        .split('\n')
        .map(line => {
            const match = line.match(/^trial (\d+): (\d+)$/);
            if(!match) return false;
            const [_, trial, score] = [...match].map(e => parseInt(e));
            return {trial, score};
        })
        .filter(e => e);

    // Declare the chart dimensions and margins.
    const width = 928;
    const height = 600;
    const marginTop = 20;
    const marginRight = 50;
    const marginBottom = 30;
    const marginLeft = 70;

    const x = d3.scaleLinear([0, d3.max(data, d => d.trial)], [marginLeft, width - marginRight]);
    const y = d3.scaleLinear([0, d3.max(data, d => d.score)], [height - marginBottom, marginTop]);

    const line = d3.line()
        .x(d => x(d.trial))
        .y(d => y(d.score));

    // x-axis
    SVGmountain.append("g")
        .attr("transform", `translate(0,${height - marginBottom})`)
        .call(d3.axisBottom(x).ticks(width / 80).tickSizeOuter(0));

    // y-axis
    SVGmountain.append("g")
        .attr("transform", `translate(${marginLeft},0)`)
        .call(d3.axisLeft(y).ticks(height / 40))
        .call(g => g.select(".domain").remove())
        .call(g => g.selectAll(".tick line").clone()
            .attr("x2", width - marginLeft - marginRight)
            .attr("stroke-opacity", 0.1))
        .call(g => g.append("text")
            .attr("x", -marginLeft)
            .attr("y", 10)
            .attr("fill", "currentColor")
            .attr("text-anchor", "start")
            .text("Score"));

    SVGmountain.append("path")
        .attr("fill", "none")
        .attr("stroke", "steelblue")
        .attr("stroke-width", 0.5)
        .attr("d", line(data));

    const dataInc = [];
    for (const dat of data) {
        const last = dataInc.slice(-1)[0] || {score: -1};
        if(dat.score > last.score) dataInc.push(dat);
    }
    SVGmountain.append("path")
        .attr("fill", "none")
        .attr("stroke", "red")
        .attr("stroke-width", 1)
        .attr("d", line(dataInc));

    eMountainD3.append(SVGmountain.node());
}

const app = new Vue({
    el: '#app',
    data: {
        tab: 'room',
        room_problemid: 65, // TODO
    },
    mounted() {
        this.submit();
    },
    methods: {
        async submit() {
            await _submit({
                problem_id: this.room_problemid,
            });
            await drawMountain({
                problem_id: this.room_problemid,
            });
        },
        updateTab(tab) {
            this.tab = tab;
            if(tab === 'room') {
                document.getElementById('canvas').classList.remove('is-hidden');
            } else {
                document.getElementById('canvas').classList.add('is-hidden');
            }
        }
    }
});