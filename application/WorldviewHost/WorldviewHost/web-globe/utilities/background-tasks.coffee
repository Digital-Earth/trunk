### *
 * BackgroundTasks - utility class to run background tasks on the main thread while try to keep FPS constant
 *
 * BackgroundTasks allow you to schedule tasks to be excuteded later using a time bound method.
 * moreover, you can log the average time it took run tasks by given each task a category.
 *
 * var tasks = BackgroundTasks()
 *
 * tasks.schedule(callback) <- schedule a generic task (default category is "task")
 * tasks.schedule(callback,"processing") <- schedule a task for a given category
 *
 * //helper function to specify a category automatically
 * var animationTasks = tasks.category("animation")
 * animationTasks.schedule(callback) <- schedule a task for "animation" category
 * animationTasks.schedule(callback,"ui") <- schedule a task for "animation/ui" category
 *
 * tasks.run(10) <- run given tasks for maximum of 10[ms]
 *
 *
 * tasks.stats["processing"].count //return how many tasks have been exectued for a given category
 * tasks.stats["processing"].totalTime //return total time in milliseconds for the given category
 * tasks.stats["processing"].averageTime //return average time in milliseconds for a task in a given category
###
class BackgroundTasks
	constructor: (@options) ->
		@tasks = []
		@stats = {}

	### *
	 * Schedule a task to be executed later 
	 * @param  {function} callback          callback function to be exectued
	 * @param  @optional {string} category  category to be used to track run time stats.
	###
	schedule: (callback,category) ->
		@tasks.push( { callback: callback, category: category || "task" } )
		return true

	### *
	 * run pending tasks for given period of time.
	 * @param  {number} milliseconds   maximum number of milliseconds to use to run pending tasks
	###
	run: (milliseconds) ->
		now = start = new Date()
		while now - start < milliseconds && @tasks.length > 0
			task = @tasks.shift()

			taskStart = now
			try
				task.callback()
			catch error
				console.error error, error.stack

			now = new Date()

			stat = @stats[task.category] or= { count:0, totalTime:0 }
			stat.count++
			stat.totalTime += now - taskStart
			stat.averageTime = stat.totalTime / stat.count

	### *
	 * create a background tasks sub category helper object
	 * @param  {string} category   sub category to specify for all schedule tasks using the helper object
	 * @return {BackgroundTasks}    helper object with the same api as BackgroundTasks
	###
	category: (category) ->
		self = this
		return {
			schedule: (callback,subCategory) ->
				if subCategory
					finalCategory = category + "/" + subCategory
				else
					finalCategory = category
				self.schedule(callback, finalCategory)
				return true

			run: (milliseconds) ->
				self.run(milliseconds)

			category: (subCategory) ->
				return self.category(category+ "/" + subCategory)
		}

module.exports = BackgroundTasks
