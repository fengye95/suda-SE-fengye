// 视频播放器控制器
class VideoPlayer {
    constructor() {
        this.videoPlayer = document.getElementById('videoPlayer');
        this.videoControls = document.getElementById('videoControls');
        this.playPauseBtn = document.getElementById('playPauseBtn');
        this.bigPlayBtn = document.getElementById('bigPlayBtn');
        this.smallPlayPauseBtn = document.getElementById('smallPlayPauseBtn');
        this.progressBar = document.getElementById('progressBar');
        this.progressFill = document.getElementById('progressFill');
        this.currentTimeEl = document.getElementById('currentTime');
        this.durationEl = document.getElementById('duration');
        
        this.controlsTimeout = null;
        this.isPlaying = false;
        
        this.init();
    }
    
    init() {
        this.bindEvents();
        this.setDemoVideo();
    }
    
    bindEvents() {
        // 视频事件
        this.videoPlayer.addEventListener('loadedmetadata', () => this.updateDuration());
        this.videoPlayer.addEventListener('timeupdate', () => this.updateProgress());
        this.videoPlayer.addEventListener('ended', () => this.onVideoEnd());
        this.videoPlayer.addEventListener('click', () => this.toggleControls());
        
        // 控制层事件
        this.videoControls.addEventListener('mousemove', () => this.showControls());
        
        // 按钮事件
        this.playPauseBtn.addEventListener('click', () => this.togglePlay());
        this.bigPlayBtn.addEventListener('click', () => this.togglePlay());
        this.smallPlayPauseBtn.addEventListener('click', () => this.togglePlay());
        this.progressBar.addEventListener('click', (e) => this.seek(e));
        
        // 其他控制按钮
        document.getElementById('fullscreenBtn').addEventListener('click', () => this.toggleFullscreen());
        document.getElementById('volumeBtn').addEventListener('click', () => this.toggleMute());
    }
    
    // 播放/暂停切换
    togglePlay() {
        if (this.videoPlayer.paused || this.videoPlayer.ended) {
            this.play();
        } else {
            this.pause();
        }
    }
    
    play() {
        this.videoPlayer.play()
            .then(() => {
                this.isPlaying = true;
                this.updatePlayButtons();
                this.bigPlayBtn.style.display = 'none';
                this.showControls();
            })
            .catch(error => {
                console.error('播放失败:', error);
                this.showError('视频播放失败，请稍后再试');
            });
    }
    
    pause() {
        this.videoPlayer.pause();
        this.isPlaying = false;
        this.updatePlayButtons();
        this.bigPlayBtn.style.display = 'flex';
        this.showControls();
    }
    
    // 更新播放按钮状态
    updatePlayButtons() {
        const playIcon = '<i class="fa fa-play"></i>';
        const pauseIcon = '<i class="fa fa-pause"></i>';
        
        if (this.isPlaying) {
            this.playPauseBtn.innerHTML = pauseIcon;
            this.smallPlayPauseBtn.innerHTML = pauseIcon;
        } else {
            this.playPauseBtn.innerHTML = playIcon;
            this.smallPlayPauseBtn.innerHTML = playIcon;
        }
    }
    
    // 控制层显示/隐藏
    showControls() {
        this.videoControls.style.opacity = '1';
        this.videoControls.classList.add('show');
        clearTimeout(this.controlsTimeout);
        
        if (this.isPlaying) {
            this.controlsTimeout = setTimeout(() => {
                this.hideControls();
            }, 5000);
        }
    }
    
    hideControls() {
        if (this.isPlaying) {
            this.videoControls.style.opacity = '0';
            this.videoControls.classList.remove('show');
        }
    }
    
    toggleControls() {
        if (this.videoControls.classList.contains('show')) {
            this.hideControls();
        } else {
            this.showControls();
        }
    }
    
    // 进度条控制
    updateProgress() {
        const percent = (this.videoPlayer.currentTime / this.videoPlayer.duration) * 100;
        this.progressFill.style.width = `${percent}%`;
        this.currentTimeEl.textContent = this.formatTime(this.videoPlayer.currentTime);
    }
    
    seek(event) {
        const rect = this.progressBar.getBoundingClientRect();
        const pos = (event.clientX - rect.left) / rect.width;
        this.videoPlayer.currentTime = pos * this.videoPlayer.duration;
    }
    
    // 视频时长更新
    updateDuration() {
        this.durationEl.textContent = this.formatTime(this.videoPlayer.duration);
    }
    
    // 视频结束处理
    onVideoEnd() {
        this.isPlaying = false;
        this.updatePlayButtons();
        this.bigPlayBtn.style.display = 'flex';
        this.showControls();
    }
    
    // 全屏切换
    toggleFullscreen() {
        if (!document.fullscreenElement) {
            document.documentElement.requestFullscreen().catch(err => {
                console.error(`全屏请求失败: ${err.message}`);
            });
        } else {
            if (document.exitFullscreen) {
                document.exitFullscreen();
            }
        }
    }
    
    // 静音切换
    toggleMute() {
        this.videoPlayer.muted = !this.videoPlayer.muted;
        const volumeBtn = document.getElementById('volumeBtn');
        if (this.videoPlayer.muted) {
            volumeBtn.innerHTML = '<i class="fa fa-volume-off"></i>';
        } else {
            volumeBtn.innerHTML = '<i class="fa fa-volume-up"></i>';
        }
    }
    
    // 工具函数
    formatTime(seconds) {
        if (isNaN(seconds)) return '00:00';
        
        const minutes = Math.floor(seconds / 60);
        seconds = Math.floor(seconds % 60);
        return `${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`;
    }
    
    showError(message) {
        alert(message);
    }
    
    // 设置演示视频
    setDemoVideo() {
        this.videoPlayer.src = 'https://storage.googleapis.com/web-dev-assets/video-and-source-tags/chrome.mp4';
    }
}

// 初始化应用
document.addEventListener('DOMContentLoaded', () => {
    const videoPlayer = new VideoPlayer();
    
    // 其他交互功能可以在这里添加
    console.log('视频播放器初始化完成');
});

// 添加JavaScript控制展开/收起
document.addEventListener('DOMContentLoaded', function() {
    const toggleButton = document.getElementById('toggleEpisodes');
    const episodesGrid = document.getElementById('episodesGrid');
    let isExpanded = false;

    toggleButton.addEventListener('click', function() {
        isExpanded = !isExpanded;
        
        if (isExpanded) {
            episodesGrid.classList.add('expanded');
            toggleButton.textContent = '收起';
        } else {
            episodesGrid.classList.remove('expanded');
            toggleButton.textContent = '查看全部';
        }
    });
});