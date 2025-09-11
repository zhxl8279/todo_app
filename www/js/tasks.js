// 任务管理功能
let currentTasks = [];

// 加载用户任务
async function loadUserTasks() {
    try {
        const response = await apiCall('/api/profile');

        if (response.status === 'success') {
            displayTasks(response.tasks || []);
            updateTaskStats(response.tasks || []);
        }
    } catch (error) {
        console.error('加载任务失败:', error);
        showMessage('加载任务失败', 'error');
    }
}

// 显示任务列表
function displayTasks(tasks) {
    currentTasks = tasks;
    const container = document.getElementById('tasks-container');

    if (tasks.length === 0) {
        container.innerHTML = '<div class="no-tasks">暂无任务</div>';
        return;
    }

    container.innerHTML = tasks.map(task => `
        <div class="task-item ${task.completed ? 'completed' : ''}" data-task-id="${task.id}">
            <div class="task-header">
                <h4 class="task-title">${escapeHtml(task.title)}</h4>
                <div class="task-status">
                    <span class="status-badge ${task.completed ? 'completed' : 'pending'}">
                        ${task.completed ? '已完成' : '待完成'}
                    </span>
                </div>
            </div>

            <div class="task-meta">
                <span>创建: ${formatDate(task.timestamp)}</span>
                ${task.datetime ? `<span>截止: ${formatDate(task.datetime)}</span>` : ''}
            </div>

            ${task.text ? `<div class="task-text">${escapeHtml(task.text)}</div>` : ''}

            <div class="task-actions">
                <button class="btn btn-sm ${task.completed ? 'btn-secondary' : 'btn-success'}"
                        onclick="toggleTask(${task.id}, ${!task.completed})">
                    ${task.completed ? '标记未完成' : '标记完成'}
                </button>
                <button class="btn btn-sm btn-warning" onclick="editTask(${task.id})">
                    编辑
                </button>
                <button class="btn btn-sm btn-danger" onclick="deleteTask(${task.id})">
                    删除
                </button>
            </div>
        </div>
    `).join('');
}

// 更新任务统计
function updateTaskStats(tasks) {
    const total = tasks.length;
    const completed = tasks.filter(task => task.completed).length;
    const pending = total - completed;
    const completionRate = total > 0 ? Math.round((completed / total) * 100) : 0;

    document.getElementById('total-tasks').textContent = total;
    document.getElementById('completed-tasks').textContent = completed;
    document.getElementById('pending-tasks').textContent = pending;
    document.getElementById('completion-rate').textContent = completionRate + '%';
}

// 添加新任务
async function addNewTask(taskData) {
    try {
        const response = await apiCall('/api/tasks', {
            method: 'POST',
            body: JSON.stringify(taskData)
        });

        if (response.status === 'success') {
            showMessage('任务添加成功', 'success');
            closeModal();
            await loadUserTasks();
        }
    } catch (error) {
        console.error('添加任务失败:', error);
        showMessage('添加任务失败', 'error');
    }
}

// 切换任务状态
async function toggleTask(taskId, completed) {
    try {
        const response = await apiCall(`/api/tasks/${taskId}`, {
            method: 'PATCH',
            body: JSON.stringify({ completed })
        });

        if (response.status === 'success') {
            showMessage('任务状态已更新', 'success');
            await loadUserTasks();
        }
    } catch (error) {
        console.error('更新任务失败:', error);
        showMessage('更新任务失败', 'error');
    }
}

// 删除任务
async function deleteTask(taskId) {
    if (!confirm('确定要删除这个任务吗？')) return;

    try {
        const response = await apiCall(`/api/tasks/${taskId}`, {
            method: 'DELETE'
        });

        if (response.status === 'success') {
            showMessage('任务已删除', 'success');
            await loadUserTasks();
        }
    } catch (error) {
        console.error('删除任务失败:', error);
        showMessage('删除任务失败', 'error');
    }
}

// 模态框控制
function showAddTaskModal() {
    document.getElementById('add-task-modal').style.display = 'block';
}

function closeModal() {
    document.getElementById('add-task-modal').style.display = 'none';
}

// 工具函数
function escapeHtml(text) {
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
}

function formatDate(dateString) {
    if (!dateString) return '--';
    const date = new Date(dateString);
    return date.toLocaleString('zh-CN');
}

function showMessage(message, type) {
    // 实现消息显示逻辑
    const messageDiv = document.createElement('div');
    messageDiv.className = `message ${type}`;
    messageDiv.textContent = message;
    document.body.appendChild(messageDiv);

    setTimeout(() => {
        messageDiv.remove();
    }, 3000);
}

// 事件监听
document.getElementById('add-task-form').addEventListener('submit', async function(e) {
    e.preventDefault();

    const taskData = {
        title: document.getElementById('task-title').value,
        text: document.getElementById('task-text').value,
        datetime: document.getElementById('task-datetime').value
    };

    await addNewTask(taskData);
});

// 点击模态框外部关闭
window.addEventListener('click', function(e) {
    const modal = document.getElementById('add-task-modal');
    if (e.target === modal) {
        closeModal();
    }
});