// SPDX-License-Identifier: GPL-3.0-or-later
//! In-memory file-name index of the user's folders.
//!
//! The index holds paths only (no file contents), so it is small (≈100 bytes
//! per entry) and cheap to rebuild. Live updates come from inotify via the
//! `notify` crate; a full rescan runs on start and every few hours to catch
//! anything missed (e.g. changes while watches were exhausted).
use std::collections::BTreeSet;
use std::path::{Path, PathBuf};
use walkdir::WalkDir;

/// Directories never indexed (noise, caches, VCS internals).
const SKIP_DIRS: &[&str] = &["node_modules", ".git", ".cache", "target", "__pycache__", ".venv", ".local", ".var"];

pub fn should_skip(path: &Path) -> bool {
    path.components().any(|c| {
        let s = c.as_os_str().to_string_lossy();
        (s.starts_with('.') && s.len() > 1) || SKIP_DIRS.contains(&s.as_ref())
    })
}

#[derive(Default)]
pub struct Index {
    entries: BTreeSet<PathBuf>,
}

impl Index {
    pub fn len(&self) -> usize {
        self.entries.len()
    }

    pub fn scan(roots: &[PathBuf], max_entries: usize) -> Index {
        let mut entries = BTreeSet::new();
        for root in roots {
            let walker = WalkDir::new(root).max_depth(10).follow_links(false).into_iter();
            for e in walker.filter_entry(|e| e.depth() == 0 || !should_skip(e.path().strip_prefix(root).unwrap_or(e.path()))) {
                if entries.len() >= max_entries {
                    break;
                }
                if let Ok(e) = e {
                    if e.depth() > 0 {
                        entries.insert(e.into_path());
                    }
                }
            }
        }
        Index { entries }
    }

    pub fn insert(&mut self, p: PathBuf) {
        self.entries.insert(p);
    }

    pub fn remove(&mut self, p: &Path) {
        // Removing a directory removes everything below it too.
        let under: Vec<PathBuf> = self.entries.range(p.to_path_buf()..).take_while(|e| e.starts_with(p)).cloned().collect();
        for e in under {
            self.entries.remove(&e);
        }
    }

    /// Ranked file-name search: exact name > prefix > word-start > substring.
    pub fn query(&self, query: &str, limit: usize) -> Vec<String> {
        let q = query.trim().to_lowercase();
        if q.chars().count() < 2 {
            return Vec::new();
        }
        let mut hits: Vec<(u32, usize, &PathBuf)> = Vec::new();
        for p in &self.entries {
            let name = match p.file_name() {
                Some(n) => n.to_string_lossy().to_lowercase(),
                None => continue,
            };
            let score = if name == q {
                400
            } else if name.starts_with(&q) {
                300
            } else if name.split(|c: char| c == ' ' || c == '-' || c == '_' || c == '.').any(|w| w.starts_with(&q)) {
                200
            } else if name.contains(&q) {
                100
            } else {
                continue;
            };
            hits.push((score, name.len(), p));
        }
        hits.sort_by(|a, b| b.0.cmp(&a.0).then(a.1.cmp(&b.1)));
        hits.into_iter().take(limit).map(|(_, _, p)| p.to_string_lossy().into_owned()).collect()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn skips_hidden_and_noise() {
        assert!(should_skip(Path::new("a/.git/config")));
        assert!(should_skip(Path::new("proj/node_modules/x")));
        assert!(should_skip(Path::new(".config/foo")));
        assert!(!should_skip(Path::new("Documents/Report.pdf")));
    }

    #[test]
    fn ranking_prefers_prefix_and_shorter_names() {
        let mut idx = Index::default();
        for p in ["/h/Documents/report-final.pdf", "/h/Documents/Q3 report.csv", "/h/Documents/report.txt", "/h/misc/myreports"] {
            idx.insert(PathBuf::from(p));
        }
        let r = idx.query("report", 10);
        assert_eq!(r[0], "/h/Documents/report.txt");
        assert_eq!(r[1], "/h/Documents/report-final.pdf");
        assert_eq!(r[2], "/h/Documents/Q3 report.csv");
        assert_eq!(r[3], "/h/misc/myreports");
    }

    #[test]
    fn short_queries_return_nothing() {
        let mut idx = Index::default();
        idx.insert(PathBuf::from("/h/a.txt"));
        assert!(idx.query("a", 10).is_empty());
    }

    #[test]
    fn removing_directory_removes_children() {
        let mut idx = Index::default();
        for p in ["/h/d", "/h/d/x.txt", "/h/d/y.txt", "/h/e.txt"] {
            idx.insert(PathBuf::from(p));
        }
        idx.remove(Path::new("/h/d"));
        assert_eq!(idx.len(), 1);
    }

    #[test]
    fn scan_walks_real_directories() {
        let dir = std::env::temp_dir().join(format!("lumend-test-{}", std::process::id()));
        std::fs::create_dir_all(dir.join("Docs/.hidden")).unwrap();
        std::fs::write(dir.join("Docs/notes.md"), "x").unwrap();
        std::fs::write(dir.join("Docs/.hidden/secret"), "x").unwrap();
        let idx = Index::scan(&[dir.clone()], 1000);
        let r = idx.query("notes", 5);
        assert_eq!(r.len(), 1);
        assert!(idx.query("secret", 5).is_empty());
        std::fs::remove_dir_all(dir).unwrap();
    }
}
