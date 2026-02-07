import React, { useState, useEffect, useRef } from 'react';
import Editor from '@monaco-editor/react';

declare global {
  interface Window {
    windowControls: {
      minimize: () => void;
      maximize: () => void;
      close: () => void;
      sendScript: (script: string) => void;
    };
  }
}

const TitleBar = () => {
  return (
    <div className="title-bar">
      <div className="title">exec_project - Roblox Executor</div>
      <div className="window-controls">
        <div className="control-btn" onClick={() => window.windowControls.minimize()}>_</div>
        <div className="control-btn" onClick={() => window.windowControls.maximize()}>□</div>
        <div className="control-btn close" onClick={() => window.windowControls.close()}>×</div>
      </div>
    </div>
  );
};

const Console = ({ logs }: { logs: { text: string; type: string }[] }) => {
  const endRef = useRef<HTMLDivElement>(null);
  
  useEffect(() => {
    endRef.current?.scrollIntoView({ behavior: 'smooth' });
  }, [logs]);

  return (
    <div className="console-area">
      {logs.map((log, i) => (
        <div key={i} className={`console-line ${log.type}`}>
          [{new Date().toLocaleTimeString()}] {log.text}
        </div>
      ))}
      <div ref={endRef} />
    </div>
  );
};

const ScriptHub = ({ onExecute }: { onExecute: (script: string) => void }) => {
  const [scripts, setScripts] = useState<any[]>([]);
  const [loading, setLoading] = useState(false);
  const [search, setSearch] = useState('');

  const fetchScripts = async (query = '') => {
    setLoading(true);
    try {
      const response = await fetch(`https://scriptblox.com/api/script/search?q=${query}&mode=free&max=10`);
      const data = await response.json();
      setScripts(data.result.scripts || []);
    } catch (error) {
      console.error('Error fetching scripts:', error);
    }
    setLoading(false);
  };

  useEffect(() => {
    fetchScripts();
  }, []);

  return (
    <div style={{ padding: '20px', display: 'flex', flexDirection: 'column', height: '100%' }}>
      <h2>Script Hub</h2>
      <div style={{ marginBottom: '20px', display: 'flex', gap: '10px' }}>
        <input 
          type="text" 
          placeholder="Search scripts..." 
          style={{ flex: 1, padding: '8px', borderRadius: '4px', border: '1px solid #444', backgroundColor: '#333', color: 'white' }}
          value={search}
          onChange={(e) => setSearch(e.target.value)}
          onKeyDown={(e) => e.key === 'Enter' && fetchScripts(search)}
        />
        <button className="btn btn-primary" onClick={() => fetchScripts(search)}>Search</button>
      </div>
      
      {loading ? <p>Loading...</p> : (
        <div style={{ flex: 1, overflowY: 'auto', display: 'grid', gridTemplateColumns: 'repeat(auto-fill, minmax(200px, 1fr))', gap: '15px' }}>
          {scripts.map((s) => (
            <div key={s._id} style={{ backgroundColor: '#252525', borderRadius: '8px', padding: '10px', display: 'flex', flexDirection: 'column' }}>
              <img 
                src={s.game.imageUrl ? (s.game.imageUrl.startsWith('http') ? s.game.imageUrl : `https://scriptblox.com${s.game.imageUrl}`) : 'https://via.placeholder.com/200x100'} 
                alt={s.title} 
                style={{ width: '100%', height: '100px', objectFit: 'cover', borderRadius: '4px' }}
              />
              <h4 style={{ margin: '10px 0 5px 0', fontSize: '14px', whiteSpace: 'nowrap', overflow: 'hidden', textOverflow: 'ellipsis' }}>{s.title}</h4>
              <p style={{ fontSize: '12px', color: '#aaa', margin: '0 0 10px 0' }}>{s.game.name}</p>
              <button 
                className="btn btn-primary" 
                style={{ marginTop: 'auto', width: '100%' }}
                onClick={() => onExecute(s.script)}
              >
                Execute
              </button>
            </div>
          ))}
        </div>
      )}
    </div>
  );
};

const App = () => {
  const [activeTab, setActiveTab] = useState('editor');
  const [script, setScript] = useState('-- Paste your script here\nprint("Hello from exec_project!")');
  const [logs, setLogs] = useState<{ text: string; type: string }[]>([]);

  const addLog = (text: string, type: string = 'info') => {
    setLogs(prev => [...prev, { text, type }]);
  };

  const handleExecute = (scriptToRun: string = script) => {
    addLog(`Attempting to execute script...`);
    if (window.windowControls && window.windowControls.sendScript) {
      window.windowControls.sendScript(scriptToRun);
      addLog(`Script sent to pipe.`, 'info');
    } else {
      addLog(`Error: Window controls not available.`, 'error');
    }
  };

  const handleClear = () => {
    setScript('');
    addLog('Editor cleared.');
  };

  return (
    <div className="app-container">
      <TitleBar />
      <div className="main-content">
        <div className="sidebar">
          <div 
            className={`sidebar-item ${activeTab === 'editor' ? 'active' : ''}`}
            onClick={() => setActiveTab('editor')}
            title="Editor"
          >
            📝
          </div>
          <div 
            className={`sidebar-item ${activeTab === 'hub' ? 'active' : ''}`}
            onClick={() => setActiveTab('hub')}
            title="Script Hub"
          >
            🌐
          </div>
          <div 
            className={`sidebar-item ${activeTab === 'settings' ? 'active' : ''}`}
            onClick={() => setActiveTab('settings')}
            title="Settings"
          >
            ⚙️
          </div>
        </div>
        
        <div className="content-area">
          <div style={{ flex: 1, display: 'flex', flexDirection: 'column', overflow: 'hidden' }}>
            {activeTab === 'editor' && (
              <>
                <div className="editor-container">
                  <Editor
                    height="100%"
                    defaultLanguage="lua"
                    theme="vs-dark"
                    value={script}
                    onChange={(value) => setScript(value || '')}
                    options={{
                      minimap: { enabled: false },
                      fontSize: 14,
                      automaticLayout: true,
                    }}
                  />
                </div>
                <div className="bottom-bar">
                  <button className="btn btn-primary" onClick={() => handleExecute()}>Execute</button>
                  <button className="btn" onClick={handleClear}>Clear</button>
                  <button className="btn">Open File</button>
                  <button className="btn">Save File</button>
                </div>
              </>
            )}
            {activeTab === 'hub' && (
              <ScriptHub onExecute={handleExecute} />
            )}
            {activeTab === 'settings' && (
              <div style={{ padding: '20px' }}>
                <h2>Settings</h2>
                <p>Coming soon: Executor preferences</p>
              </div>
            )}
          </div>
          <Console logs={logs} />
        </div>
      </div>
    </div>
  );
};

export default App;