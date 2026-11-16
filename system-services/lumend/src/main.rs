// SPDX-License-Identifier: GPL-3.0-or-later
//! lumend — LUMEN session daemon.
//!
//! Serves `org.lumen.Search` on the session bus (object `/Search`):
//!   Query(s query, u limit) -> as paths
//!   Reindex()
//!   IndexedCount (property, u)
//! Spotlight in lumen-shell uses it when present and falls back to its own
//! in-process scan otherwise.
mod index;

use index::{should_skip, Index};
use notify::{EventKind, RecursiveMode, Watcher};
use std::path::PathBuf;
use std::sync::Arc;
use std::time::Duration;
use tokio::sync::RwLock;
use zbus::{connection, interface};

const MAX_ENTRIES: usize = 500_000;

fn roots() -> Vec<PathBuf> {
    let mut r: Vec<PathBuf> = [dirs::desktop_dir(), dirs::document_dir(), dirs::download_dir(), dirs::picture_dir(),
                                dirs::audio_dir(), dirs::video_dir()]
        .into_iter()
        .flatten()
        .filter(|p| p.is_dir())
        .collect();
    r.sort();
    r.dedup();
    if r.is_empty() {
        if let Some(h) = dirs::home_dir() {
            r.push(h);
        }
    }
    r
}

struct Search {
    index: Arc<RwLock<Index>>,
    roots: Vec<PathBuf>,
}

#[interface(name = "org.lumen.Search")]
impl Search {
    async fn query(&self, query: String, limit: u32) -> Vec<String> {
        self.index.read().await.query(&query, limit.min(200) as usize)
    }

    async fn reindex(&self) {
        let roots = self.roots.clone();
        let index = self.index.clone();
        tokio::task::spawn_blocking(move || {
            let fresh = Index::scan(&roots, MAX_ENTRIES);
            let n = fresh.len();
            *index.blocking_write() = fresh;
            log::info!("reindexed: {n} entries");
        });
    }

    #[zbus(property)]
    async fn indexed_count(&self) -> u32 {
        self.index.read().await.len() as u32
    }
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("info")).init();
    let roots = roots();
    let index = Arc::new(RwLock::new(Index::default()));

    // Initial scan off the async runtime.
    {
        let (roots, index) = (roots.clone(), index.clone());
        tokio::task::spawn_blocking(move || {
            let idx = Index::scan(&roots, MAX_ENTRIES);
            log::info!("indexed {} entries under {:?}", idx.len(), roots);
            *index.blocking_write() = idx;
        });
    }

    // Live updates from inotify.
    let (tx, mut rx) = tokio::sync::mpsc::unbounded_channel();
    let mut watcher = notify::recommended_watcher(move |res: notify::Result<notify::Event>| {
        if let Ok(ev) = res {
            let _ = tx.send(ev);
        }
    })?;
    for r in &roots {
        if let Err(e) = watcher.watch(r, RecursiveMode::Recursive) {
            log::warn!("cannot watch {}: {e}", r.display());
        }
    }
    {
        let index = index.clone();
        let roots = roots.clone();
        tokio::spawn(async move {
            while let Some(ev) = rx.recv().await {
                let mut idx = index.write().await;
                for p in ev.paths {
                    let rel_skip = roots.iter().find_map(|r| p.strip_prefix(r).ok()).map(should_skip).unwrap_or(true);
                    if rel_skip {
                        continue;
                    }
                    match ev.kind {
                        EventKind::Create(_) => idx.insert(p),
                        EventKind::Remove(_) => idx.remove(&p),
                        EventKind::Modify(notify::event::ModifyKind::Name(_)) => {
                            if p.exists() { idx.insert(p) } else { idx.remove(&p) }
                        }
                        _ => {}
                    }
                }
            }
        });
    }
    // Periodic safety rescan.
    {
        let (index, roots) = (index.clone(), roots.clone());
        tokio::spawn(async move {
            let mut t = tokio::time::interval(Duration::from_secs(6 * 3600));
            t.tick().await;
            loop {
                t.tick().await;
                let (r, i) = (roots.clone(), index.clone());
                let _ = tokio::task::spawn_blocking(move || {
                    let fresh = Index::scan(&r, MAX_ENTRIES);
                    *i.blocking_write() = fresh;
                })
                .await;
            }
        });
    }

    let _conn = connection::Builder::session()?
        .name("org.lumen.Search")?
        .serve_at("/Search", Search { index, roots })?
        .build()
        .await?;
    log::info!("org.lumen.Search ready");
    tokio::signal::ctrl_c().await?;
    drop(watcher);
    Ok(())
}
