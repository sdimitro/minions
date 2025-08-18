<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Cache Associativity Visualization</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background-color: #f9fafb;
            color: #374151;
            line-height: 1.6;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
            padding: 2rem;
            background: white;
            min-height: 100vh;
        }
        
        .title {
            font-size: 2.5rem;
            font-weight: bold;
            text-align: center;
            margin-bottom: 2rem;
            color: #1f2937;
        }
        
        .section {
            margin-bottom: 2rem;
        }
        
        .section-title {
            font-size: 1.5rem;
            font-weight: 600;
            margin-bottom: 1rem;
            color: #1f2937;
        }
        
        .cache-buttons {
            display: flex;
            gap: 1rem;
            flex-wrap: wrap;
        }
        
        .cache-btn {
            padding: 0.75rem 1.5rem;
            border: none;
            border-radius: 0.5rem;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.2s;
        }
        
        .cache-btn.active {
            background-color: #2563eb;
            color: white;
        }
        
        .cache-btn:not(.active) {
            background-color: #e5e7eb;
            color: #374151;
        }
        
        .cache-btn:hover {
            transform: translateY(-1px);
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        
        .cache-btn:not(.active):hover {
            background-color: #d1d5db;
        }
        
        .description-box {
            background-color: #eff6ff;
            padding: 1.5rem;
            border-radius: 0.75rem;
            border-left: 4px solid #2563eb;
        }
        
        .description-title {
            font-weight: 600;
            margin-bottom: 0.5rem;
        }
        
        .description-text {
            font-size: 0.95rem;
            color: #4b5563;
        }
        
        .input-group {
            display: flex;
            align-items: center;
            gap: 1rem;
            flex-wrap: wrap;
            margin-bottom: 1rem;
        }
        
        .input-label {
            font-weight: 500;
        }
        
        .input-field {
            border: 2px solid #d1d5db;
            border-radius: 0.5rem;
            padding: 0.5rem 0.75rem;
            width: 80px;
            font-size: 1rem;
        }
        
        .input-field:focus {
            outline: none;
            border-color: #2563eb;
            box-shadow: 0 0 0 3px rgba(37, 99, 235, 0.1);
        }
        
        .access-btn {
            background-color: #16a34a;
            color: white;
            border: none;
            padding: 0.75rem 1.5rem;
            border-radius: 0.5rem;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.2s;
        }
        
        .access-btn:hover {
            background-color: #15803d;
            transform: translateY(-1px);
        }
        
        .quick-access {
            display: flex;
            align-items: center;
            gap: 0.5rem;
            flex-wrap: wrap;
        }
        
        .quick-btn {
            background-color: #e5e7eb;
            color: #374151;
            border: none;
            padding: 0.5rem 0.75rem;
            border-radius: 0.375rem;
            font-size: 0.875rem;
            cursor: pointer;
            transition: all 0.2s;
        }
        
        .quick-btn:hover {
            background-color: #d1d5db;
            transform: translateY(-1px);
        }
        
        .cache-grid {
            display: flex;
            flex-wrap: wrap;
            gap: 0.75rem;
        }
        
        .cache-block {
            width: 90px;
            height: 90px;
            border: 2px solid #d1d5db;
            border-radius: 0.5rem;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            font-size: 0.8rem;
            transition: all 0.3s;
            cursor: pointer;
        }
        
        .cache-block:hover {
            transform: scale(1.05);
            box-shadow: 0 4px 12px rgba(0,0,0,0.15);
        }
        
        .cache-block.empty {
            background-color: #f3f4f6;
            color: #6b7280;
        }
        
        .block-label {
            font-weight: 600;
            margin-bottom: 0.25rem;
        }
        
        .block-info {
            font-size: 0.7rem;
            text-align: center;
        }
        
        .set-container {
            border: 2px solid #9ca3af;
            border-radius: 0.5rem;
            padding: 1rem;
            margin-bottom: 1rem;
        }
        
        .set-title {
            font-size: 0.9rem;
            font-weight: 600;
            margin-bottom: 0.75rem;
            color: #374151;
        }
        
        .set-blocks {
            display: flex;
            gap: 0.5rem;
        }
        
        .history-container {
            background-color: #f9fafb;
            border-radius: 0.75rem;
            padding: 1.5rem;
            max-height: 200px;
            overflow-y: auto;
        }
        
        .history-empty {
            color: #6b7280;
            font-style: italic;
            text-align: center;
        }
        
        .history-entry {
            padding: 0.75rem;
            margin-bottom: 0.5rem;
            border-radius: 0.5rem;
            font-size: 0.9rem;
            border-left: 4px solid;
        }
        
        .history-entry.hit {
            background-color: #dcfce7;
            border-color: #16a34a;
            color: #166534;
        }
        
        .history-entry.miss {
            background-color: #fee2e2;
            border-color: #dc2626;
            color: #991b1b;
        }
        
        .legend-container {
            background-color: #f9fafb;
            padding: 1.5rem;
            border-radius: 0.75rem;
        }
        
        .legend-title {
            font-weight: 600;
            margin-bottom: 1rem;
        }
        
        .legend-list {
            list-style: none;
        }
        
        .legend-item {
            margin-bottom: 0.75rem;
            font-size: 0.9rem;
        }
        
        .legend-item strong {
            color: #1f2937;
        }
        
        /* Color classes for cache blocks */
        .color-blue { background-color: #dbeafe; color: #1e40af; }
        .color-green { background-color: #bbf7d0; color: #166534; }
        .color-yellow { background-color: #fef3c7; color: #92400e; }
        .color-purple { background-color: #e9d5ff; color: #7c3aed; }
        .color-pink { background-color: #fbcfe8; color: #be185d; }
        .color-indigo { background-color: #c7d2fe; color: #4338ca; }
        .color-red { background-color: #fecaca; color: #dc2626; }
        .color-orange { background-color: #fed7aa; color: #ea580c; }
        
        @media (max-width: 768px) {
            .container {
                padding: 1rem;
            }
            
            .title {
                font-size: 2rem;
            }
            
            .cache-buttons {
                flex-direction: column;
            }
            
            .cache-btn {
                width: 100%;
            }
            
            .input-group {
                flex-direction: column;
                align-items: flex-start;
            }
            
            .cache-grid {
                justify-content: center;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1 class="title">Cache Associativity Visualization</h1>
        
        <!-- Cache Type Selection -->
        <div class="section">
            <h2 class="section-title">Select Cache Type:</h2>
            <div class="cache-buttons">
                <button class="cache-btn active" data-cache="direct">Direct-Mapped</button>
                <button class="cache-btn" data-cache="fully">Fully Associative</button>
                <button class="cache-btn" data-cache="set">Set-Associative (2-way)</button>
            </div>
        </div>
        
        <!-- Description -->
        <div class="section">
            <div class="description-box">
                <div class="description-title">How it works:</div>
                <div class="description-text" id="description">
                    Each memory address maps to exactly one cache block using: block_index = address % cache_size
                </div>
            </div>
        </div>
        
        <!-- Memory Access Input -->
        <div class="section">
            <h2 class="section-title">Memory Access Simulation:</h2>
            <div class="input-group">
                <label class="input-label">Memory Address (0-31):</label>
                <input type="number" class="input-field" id="memoryAddress" min="0" max="31" value="0">
                <button class="access-btn" id="accessBtn">Access Memory</button>
            </div>
            <div class="quick-access">
                <span style="font-weight: 500; font-size: 0.9rem;">Quick access:</span>
                <button class="quick-btn" data-addr="0">0</button>
                <button class="quick-btn" data-addr="1">1</button>
                <button class="quick-btn" data-addr="8">8</button>
                <button class="quick-btn" data-addr="9">9</button>
                <button class="quick-btn" data-addr="16">16</button>
                <button class="quick-btn" data-addr="17">17</button>
                <button class="quick-btn" data-addr="24">24</button>
                <button class="quick-btn" data-addr="25">25</button>
            </div>
        </div>
        
        <!-- Cache Visualization -->
        <div class="section">
            <h2 class="section-title">Cache State (8 blocks):</h2>
            <div id="cacheContainer"></div>
        </div>
        
        <!-- Access History -->
        <div class="section">
            <h2 class="section-title">Recent Access History:</h2>
            <div class="history-container">
                <div class="history-empty" id="historyEmpty">No accesses yet</div>
                <div id="historyList" style="display: none;"></div>
            </div>
        </div>
        
        <!-- Legend -->
        <div class="section">
            <div class="legend-container">
                <div class="legend-title">Key Differences:</div>
                <ul class="legend-list">
                    <li class="legend-item">
                        <strong>Direct-Mapped:</strong> Fastest, simplest hardware, but highest conflict miss rate
                    </li>
                    <li class="legend-item">
                        <strong>Fully Associative:</strong> Lowest miss rate, most flexible, but complex and slow hardware
                    </li>
                    <li class="legend-item">
                        <strong>Set-Associative:</strong> Good balance between speed and miss rate, commonly used in real processors
                    </li>
                </ul>
            </div>
        </div>
    </div>

    <script>
        class CacheSimulator {
            constructor() {
                this.CACHE_SIZE = 8;
                this.MEMORY_SIZE = 32;
                this.SET_SIZE = 2;
                this.selectedCache = 'direct';
                this.cacheState = {};
                this.accessHistory = [];
                this.colors = ['blue', 'green', 'yellow', 'purple', 'pink', 'indigo', 'red', 'orange'];
                
                this.initializeEventListeners();
                this.updateDescription();
                this.renderCache();
            }
            
            initializeEventListeners() {
                // Cache type buttons
                document.querySelectorAll('.cache-btn').forEach(btn => {
                    btn.addEventListener('click', (e) => {
                        this.selectCacheType(e.target.dataset.cache);
                    });
                });
                
                // Access button
                document.getElementById('accessBtn').addEventListener('click', () => {
                    const address = parseInt(document.getElementById('memoryAddress').value) || 0;
                    this.simulateAccess(address);
                });
                
                // Quick access buttons
                document.querySelectorAll('.quick-btn').forEach(btn => {
                    btn.addEventListener('click', (e) => {
                        const address = parseInt(e.target.dataset.addr);
                        document.getElementById('memoryAddress').value = address;
                        this.simulateAccess(address);
                    });
                });
                
                // Enter key on input
                document.getElementById('memoryAddress').addEventListener('keypress', (e) => {
                    if (e.key === 'Enter') {
                        const address = parseInt(e.target.value) || 0;
                        this.simulateAccess(address);
                    }
                });
            }
            
            selectCacheType(cacheType) {
                // Update button states
                document.querySelectorAll('.cache-btn').forEach(btn => {
                    btn.classList.remove('active');
                });
                document.querySelector(`[data-cache="${cacheType}"]`).classList.add('active');
                
                // Reset cache state
                this.selectedCache = cacheType;
                this.cacheState = {};
                this.accessHistory = [];
                
                this.updateDescription();
                this.renderCache();
                this.updateHistory();
            }
            
            updateDescription() {
                const descriptions = {
                    direct: 'Each memory address maps to exactly one cache block using: block_index = address % cache_size',
                    fully: 'Any memory address can be placed in any cache block. Provides maximum flexibility but requires more complex hardware.',
                    set: `Cache is divided into ${this.CACHE_SIZE/this.SET_SIZE} sets, each with ${this.SET_SIZE} blocks. Address maps to a set, then can go in any block within that set.`
                };
                
                document.getElementById('description').textContent = descriptions[this.selectedCache];
            }
            
            getColor(tag) {
                return this.colors[tag % this.colors.length];
            }
            
            simulateAccess(address) {
                if (address < 0 || address >= this.MEMORY_SIZE) return;
                
                let hit = false;
                let replacedBlock = null;
                let cacheIndex = null;
                let newState = {...this.cacheState};
                
                if (this.selectedCache === 'direct') {
                    cacheIndex = address % this.CACHE_SIZE;
                    const tag = Math.floor(address / this.CACHE_SIZE);
                    
                    if (newState[cacheIndex] && newState[cacheIndex].tag === tag) {
                        hit = true;
                    } else {
                        if (newState[cacheIndex]) {
                            replacedBlock = newState[cacheIndex];
                        }
                        newState[cacheIndex] = { 
                            tag, 
                            address, 
                            color: this.getColor(tag) 
                        };
                    }
                } else if (this.selectedCache === 'fully') {
                    const tag = address;
                    let foundIndex = null;
                    
                    // Check if already in cache
                    for (let i = 0; i < this.CACHE_SIZE; i++) {
                        if (newState[i] && newState[i].tag === tag) {
                            hit = true;
                            foundIndex = i;
                            break;
                        }
                    }
                    
                    if (!hit) {
                        // Find empty slot or use first slot
                        let targetIndex = 0;
                        for (let i = 0; i < this.CACHE_SIZE; i++) {
                            if (!newState[i]) {
                                targetIndex = i;
                                break;
                            }
                        }
                        
                        if (newState[targetIndex]) {
                            replacedBlock = newState[targetIndex];
                        }
                        newState[targetIndex] = { 
                            tag, 
                            address, 
                            color: this.getColor(tag) 
                        };
                        cacheIndex = targetIndex;
                    } else {
                        cacheIndex = foundIndex;
                    }
                } else if (this.selectedCache === 'set') {
                    const numSets = this.CACHE_SIZE / this.SET_SIZE;
                    const setIndex = address % numSets;
                    const tag = Math.floor(address / numSets);
                    
                    // Check if in the set
                    for (let way = 0; way < this.SET_SIZE; way++) {
                        const index = setIndex * this.SET_SIZE + way;
                        if (newState[index] && newState[index].tag === tag) {
                            hit = true;
                            cacheIndex = index;
                            break;
                        }
                    }
                    
                    if (!hit) {
                        // Find empty way in set or replace first way
                        let targetWay = 0;
                        for (let way = 0; way < this.SET_SIZE; way++) {
                            const index = setIndex * this.SET_SIZE + way;
                            if (!newState[index]) {
                                targetWay = way;
                                break;
                            }
                        }
                        
                        cacheIndex = setIndex * this.SET_SIZE + targetWay;
                        if (newState[cacheIndex]) {
                            replacedBlock = newState[cacheIndex];
                        }
                        newState[cacheIndex] = { 
                            tag, 
                            address, 
                            color: this.getColor(tag) 
                        };
                    }
                }
                
                this.cacheState = newState;
                this.accessHistory.push({
                    address,
                    hit,
                    replacedBlock,
                    cacheIndex,
                    type: this.selectedCache
                });
                
                // Keep only last 10 entries
                if (this.accessHistory.length > 10) {
                    this.accessHistory = this.accessHistory.slice(-10);
                }
                
                this.renderCache();
                this.updateHistory();
            }
            
            renderCache() {
                const container = document.getElementById('cacheContainer');
                container.innerHTML = '';
                
                if (this.selectedCache === 'set') {
                    // Render as sets
                    const numSets = this.CACHE_SIZE / this.SET_SIZE;
                    for (let set = 0; set < numSets; set++) {
                        const setDiv = document.createElement('div');
                        setDiv.className = 'set-container';
                        
                        const setTitle = document.createElement('div');
                        setTitle.className = 'set-title';
                        setTitle.textContent = `Set ${set}`;
                        setDiv.appendChild(setTitle);
                        
                        const setBlocks = document.createElement('div');
                        setBlocks.className = 'set-blocks';
                        
                        for (let way = 0; way < this.SET_SIZE; way++) {
                            const index = set * this.SET_SIZE + way;
                            const block = this.cacheState[index];
                            
                            const blockDiv = document.createElement('div');
                            blockDiv.className = `cache-block ${block ? `color-${block.color}` : 'empty'}`;
                            
                            const label = document.createElement('div');
                            label.className = 'block-label';
                            label.textContent = `Way ${way}`;
                            blockDiv.appendChild(label);
                            
                            if (block) {
                                const info = document.createElement('div');
                                info.className = 'block-info';
                                info.innerHTML = `Addr: ${block.address}<br>Tag: ${block.tag}`;
                                blockDiv.appendChild(info);
                            }
                            
                            setBlocks.appendChild(blockDiv);
                        }
                        
                        setDiv.appendChild(setBlocks);
                        container.appendChild(setDiv);
                    }
                } else {
                    // Render as simple blocks
                    const gridDiv = document.createElement('div');
                    gridDiv.className = 'cache-grid';
                    
                    for (let i = 0; i < this.CACHE_SIZE; i++) {
                        const block = this.cacheState[i];
                        
                        const blockDiv = document.createElement('div');
                        blockDiv.className = `cache-block ${block ? `color-${block.color}` : 'empty'}`;
                        
                        const label = document.createElement('div');
                        label.className = 'block-label';
                        label.textContent = `Block ${i}`;
                        blockDiv.appendChild(label);
                        
                        if (block) {
                            const info = document.createElement('div');
                            info.className = 'block-info';
                            info.innerHTML = `Addr: ${block.address}<br>Tag: ${block.tag}`;
                            blockDiv.appendChild(info);
                        }
                        
                        gridDiv.appendChild(blockDiv);
                    }
                    
                    container.appendChild(gridDiv);
                }
            }
            
            updateHistory() {
                const historyEmpty = document.getElementById('historyEmpty');
                const historyList = document.getElementById('historyList');
                
                if (this.accessHistory.length === 0) {
                    historyEmpty.style.display = 'block';
                    historyList.style.display = 'none';
                } else {
                    historyEmpty.style.display = 'none';
                    historyList.style.display = 'block';
                    
                    historyList.innerHTML = '';
                    
                    // Show most recent first
                    const recentHistory = [...this.accessHistory].reverse();
                    
                    recentHistory.forEach(access => {
                        const entryDiv = document.createElement('div');
                        entryDiv.className = `history-entry ${access.hit ? 'hit' : 'miss'}`;
                        
                        let text = `Address ${access.address}: ${access.hit ? 'HIT' : 'MISS'}`;
                        if (access.cacheIndex !== null) {
                            text += ` → Block ${access.cacheIndex}`;
                        }
                        if (access.replacedBlock) {
                            text += ` (replaced addr ${access.replacedBlock.address})`;
                        }
                        
                        entryDiv.textContent = text;
                        historyList.appendChild(entryDiv);
                    });
                }
            }
        }
        
        // Initialize the simulator when the page loads
        document.addEventListener('DOMContentLoaded', () => {
            new CacheSimulator();
        });
    </script>
</body>
</html>
