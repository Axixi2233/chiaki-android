// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL

package com.metallic.chiaki.main

import android.animation.Animator
import android.animation.AnimatorSet
import android.animation.ObjectAnimator
import android.animation.PropertyValuesHolder
import android.content.Context
import android.content.res.Resources
import android.util.AttributeSet
import android.view.View
import android.view.ViewGroup
import android.view.animation.AccelerateInterpolator
import android.view.animation.DecelerateInterpolator
import androidx.coordinatorlayout.widget.CoordinatorLayout
import androidx.core.animation.addListener
import androidx.core.view.children
import androidx.core.view.isInvisible
import androidx.core.view.isVisible
import com.google.android.material.floatingactionbutton.FloatingActionButton
import com.google.android.material.transformation.ExpandableTransformationBehavior
import com.metallic.chiaki.R

// see https://github.com/lcdsmao/ExpandableFABExample

class FloatingActionButtonSpeedDialBehavior @JvmOverloads constructor(context: Context? = null, attrs: AttributeSet? = null) : ExpandableTransformationBehavior(context, attrs)
{
	companion object
	{
		private const val DELAY = 30L
		private const val DURATION = 150L
	}

	override fun layoutDependsOn(parent: CoordinatorLayout, child: View, dependency: View)
		= dependency is FloatingActionButton && child is ViewGroup

	override fun onCreateExpandedStateChangeAnimation(dependency: View, child: View, expanded: Boolean, isAnimating: Boolean): AnimatorSet
		= if(child !is ViewGroup)
			AnimatorSet()
		else
			AnimatorSet().also {
				it.playTogether(listOf(
					if(expanded)
						createExpandAnimation(child, isAnimating)
					else
						createCollapseAnimation(child)

				))
				it.addListener(
					onStart = {
						if(expanded)
							child.isVisible = true
					},
					onEnd = {
						if(!expanded)
							child.isInvisible = true
					}
				)
			}

	private fun offset(resources: Resources) = resources.getDimension(R.dimen.floating_action_button_speed_dial_anim_offset)

	private fun createExpandAnimation(child: ViewGroup, currentlyAnimating: Boolean): Animator
	{
		if(!currentlyAnimating)
		{
			child.children.forEach {
				it.alpha = 0f
				it.translationY = this.offset(child.resources)
			}
		}

		val translationYHolder = PropertyValuesHolder.ofFloat(View.TRANSLATION_Y, 0f)
		val alphaHolder = PropertyValuesHolder.ofFloat(View.ALPHA, 1f)

		val animators = child.children.mapIndexed { index, view ->
			ObjectAnimator.ofPropertyValuesHolder(
				view,
				translationYHolder,
				alphaHolder
			).apply {
				duration = DURATION
				startDelay = (child.childCount - index - 1) * DELAY
				interpolator = DecelerateInterpolator()
			}
		}.toList()

		return AnimatorSet().apply {
			playTogether(animators)
		}
	}

	private fun createCollapseAnimation(child: ViewGroup): Animator
	{
		val translationYHolder = PropertyValuesHolder.ofFloat(View.TRANSLATION_Y, this.offset(child.resources))
		val alphaHolder = PropertyValuesHolder.ofFloat(View.ALPHA, 0f)

		val animators = child.children.mapIndexed { index, view ->
			ObjectAnimator.ofPropertyValuesHolder(
				view,
				translationYHolder,
				alphaHolder
			).apply {
				duration = DURATION
				startDelay = index * DELAY
				interpolator = AccelerateInterpolator()
			}
		}.toList()

		return AnimatorSet().apply {
			playTogether(animators)
		}
	}
}
