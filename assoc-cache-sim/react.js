import React, { useState, useEffect } from 'react';

const CacheVisualization = () => {
  const [selectedCache, setSelectedCache] = useState('direct');
  const [memoryAddress, setMemoryAddress] = useState(0);
  const [cacheState, setCacheState] = useState({});
  const [accessHistory, setAccessHistory] = useState([]);

  // Cache configurations
  const CACHE_SIZE = 8; // 8 blocks for simplicity
  const MEMORY_SIZE = 32; // 32 memory blocks
  const SET_SIZE = 2; // for set-associative

  // Initialize cache
  useEffect(() => {
    setCacheState({});
    setAccessHistory([]);
  }, [selectedCache]);

  const getColor = (tag) => {
    const colors = ['bg-blue-200', 'bg-green-200', 'bg-yellow-200', 'bg-purple-200', 'bg-pink-200', 'bg-indigo-200', 'bg-red-200', 'bg-orange-200'];
    return colors[tag % colors.length];
  };

  const simulateAccess = (address) => {
    let hit = false;
    let replacedBlock = null;
    let cacheIndex = null;
    let newState = { ...cacheState };

    if (selectedCache === 'direct') {
      // Direct-mapped: address % cache_size
      cacheIndex = address % CACHE_SIZE;
      const tag = Math.floor(address / CACHE_SIZE);
      
      if (newState[cacheIndex] && newState[cacheIndex].tag === tag) {
        hit = true;
      } else {
        if (newState[cacheIndex]) {
          replacedBlock = newState[cacheIndex];
        }
        newState[cacheIndex] = { tag, address, color: getColor(tag) };
      }
    } else if (selectedCache === 'fully') {
      // Fully associative: can go anywhere
      const tag = address;
      let foundIndex = null;
      
      // Check if already in cache
      for (let i = 0; i < CACHE_SIZE; i++) {
        if (newState[i] && newState[i].tag === tag) {
          hit = true;
          foundIndex = i;
          break;
        }
      }
      
      if (!hit) {
        // Find empty slot or use LRU (simplified: just use first available or replace index 0)
        let targetIndex = 0;
        for (let i = 0; i < CACHE_SIZE; i++) {
          if (!newState[i]) {
            targetIndex = i;
            break;
          }
        }
        
        if (newState[targetIndex]) {
          replacedBlock = newState[targetIndex];
        }
        newState[targetIndex] = { tag, address, color: getColor(tag) };
        cacheIndex = targetIndex;
      } else {
        cacheIndex = foundIndex;
      }
    } else if (selectedCache === 'set') {
      // Set-associative: 4 sets of 2 ways each
      const numSets = CACHE_SIZE / SET_SIZE;
      const setIndex = address % numSets;
      const tag = Math.floor(address / numSets);
      
      // Check if in the set
      for (let way = 0; way < SET_SIZE; way++) {
        const index = setIndex * SET_SIZE + way;
        if (newState[index] && newState[index].tag === tag) {
          hit = true;
          cacheIndex = index;
          break;
        }
      }
      
      if (!hit) {
        // Find empty way in set or replace first way
        let targetWay = 0;
        for (let way = 0; way < SET_SIZE; way++) {
          const index = setIndex * SET_SIZE + way;
          if (!newState[index]) {
            targetWay = way;
            break;
          }
        }
        
        cacheIndex = setIndex * SET_SIZE + targetWay;
        if (newState[cacheIndex]) {
          replacedBlock = newState[cacheIndex];
        }
        newState[cacheIndex] = { tag, address, color: getColor(tag) };
      }
    }

    setCacheState(newState);
    setAccessHistory(prev => [...prev.slice(-9), {
      address,
      hit,
      replacedBlock,
      cacheIndex,
      type: selectedCache
    }]);
  };

  const renderCache = () => {
    const blocks = [];
    
    if (selectedCache === 'set') {
      // Render as sets
      const numSets = CACHE_SIZE / SET_SIZE;
      for (let set = 0; set < numSets; set++) {
        blocks.push(
          <div key={`set-${set}`} className="border-2 border-gray-400 p-2 mb-2">
            <div className="text-sm font-semibold mb-1">Set {set}</div>
            <div className="flex gap-1">
              {[...Array(SET_SIZE)].map((_, way) => {
                const index = set * SET_SIZE + way;
                const block = cacheState[index];
                return (
                  <div
                    key={index}
                    className={`w-16 h-16 border border-gray-300 flex flex-col items-center justify-center text-xs ${
                      block ? block.color : 'bg-gray-100'
                    }`}
                  >
                    <div>Way {way}</div>
                    {block && (
                      <>
                        <div>Addr: {block.address}</div>
                        <div>Tag: {block.tag}</div>
                      </>
                    )}
                  </div>
                );
              })}
            </div>
          </div>
        );
      }
    } else {
      // Render as simple blocks
      for (let i = 0; i < CACHE_SIZE; i++) {
        const block = cacheState[i];
        blocks.push(
          <div
            key={i}
            className={`w-20 h-20 border border-gray-300 flex flex-col items-center justify-center text-xs ${
              block ? block.color : 'bg-gray-100'
            }`}
          >
            <div>Block {i}</div>
            {block && (
              <>
                <div>Addr: {block.address}</div>
                <div>Tag: {block.tag}</div>
              </>
            )}
          </div>
        );
      }
    }
    
    return blocks;
  };

  const getCacheDescription = () => {
    switch (selectedCache) {
      case 'direct':
        return 'Each memory address maps to exactly one cache block using: block_index = address % cache_size';
      case 'fully':
        return 'Any memory address can be placed in any cache block. Provides maximum flexibility but requires more complex hardware.';
      case 'set':
        return `Cache is divided into ${CACHE_SIZE/SET_SIZE} sets, each with ${SET_SIZE} blocks. Address maps to a set, then can go in any block within that set.`;
      default:
        return '';
    }
  };

  return (
    <div className="p-6 max-w-6xl mx-auto bg-white">
      <h1 className="text-3xl font-bold mb-6 text-center">Cache Associativity Visualization</h1>
      
      {/* Cache Type Selection */}
      <div className="mb-6">
        <h2 className="text-xl font-semibold mb-3">Select Cache Type:</h2>
        <div className="flex gap-4">
          {[
            { id: 'direct', name: 'Direct-Mapped' },
            { id: 'fully', name: 'Fully Associative' },
            { id: 'set', name: 'Set-Associative (2-way)' }
          ].map(cache => (
            <button
              key={cache.id}
              onClick={() => setSelectedCache(cache.id)}
              className={`px-4 py-2 rounded font-medium ${
                selectedCache === cache.id
                  ? 'bg-blue-600 text-white'
                  : 'bg-gray-200 text-gray-700 hover:bg-gray-300'
              }`}
            >
              {cache.name}
            </button>
          ))}
        </div>
      </div>

      {/* Description */}
      <div className="mb-6 p-4 bg-blue-50 rounded-lg">
        <h3 className="font-semibold mb-2">How it works:</h3>
        <p className="text-sm">{getCacheDescription()}</p>
      </div>

      {/* Memory Address Input */}
      <div className="mb-6">
        <h2 className="text-xl font-semibold mb-3">Memory Access Simulation:</h2>
        <div className="flex items-center gap-4">
          <label className="font-medium">Memory Address (0-{MEMORY_SIZE-1}):</label>
          <input
            type="number"
            min="0"
            max={MEMORY_SIZE-1}
            value={memoryAddress}
            onChange={(e) => setMemoryAddress(parseInt(e.target.value) || 0)}
            className="border border-gray-300 rounded px-3 py-1 w-20"
          />
          <button
            onClick={() => simulateAccess(memoryAddress)}
            className="bg-green-600 text-white px-4 py-2 rounded hover:bg-green-700"
          >
            Access Memory
          </button>
        </div>
        
        {/* Quick access buttons */}
        <div className="mt-3">
          <span className="text-sm font-medium">Quick access: </span>
          {[0, 1, 8, 9, 16, 17, 24, 25].map(addr => (
            <button
              key={addr}
              onClick={() => {
                setMemoryAddress(addr);
                simulateAccess(addr);
              }}
              className="ml-2 px-2 py-1 bg-gray-200 text-gray-700 rounded text-sm hover:bg-gray-300"
            >
              {addr}
            </button>
          ))}
        </div>
      </div>

      {/* Cache Visualization */}
      <div className="mb-6">
        <h2 className="text-xl font-semibold mb-3">Cache State ({CACHE_SIZE} blocks):</h2>
        <div className={selectedCache === 'set' ? '' : 'flex flex-wrap gap-2'}>
          {renderCache()}
        </div>
      </div>

      {/* Access History */}
      <div className="mb-6">
        <h2 className="text-xl font-semibold mb-3">Recent Access History:</h2>
        <div className="bg-gray-50 p-4 rounded-lg max-h-40 overflow-y-auto">
          {accessHistory.length === 0 ? (
            <p className="text-gray-500 italic">No accesses yet</p>
          ) : (
            <div className="space-y-1">
              {accessHistory.slice().reverse().map((access, idx) => (
                <div key={idx} className={`text-sm p-2 rounded ${access.hit ? 'bg-green-100' : 'bg-red-100'}`}>
                  <strong>Address {access.address}:</strong> {access.hit ? 'HIT' : 'MISS'} 
                  {access.cacheIndex !== null && ` → Block ${access.cacheIndex}`}
                  {access.replacedBlock && ` (replaced addr ${access.replacedBlock.address})`}
                </div>
              ))}
            </div>
          )}
        </div>
      </div>

      {/* Legend */}
      <div className="mt-6 p-4 bg-gray-50 rounded-lg">
        <h3 className="font-semibold mb-2">Key Differences:</h3>
        <ul className="text-sm space-y-1">
          <li><strong>Direct-Mapped:</strong> Fastest, simplest hardware, but highest conflict miss rate</li>
          <li><strong>Fully Associative:</strong> Lowest miss rate, most flexible, but complex and slow hardware</li>
          <li><strong>Set-Associative:</strong> Good balance between speed and miss rate, commonly used in real processors</li>
        </ul>
      </div>
    </div>
  );
};

export default CacheVisualization;
