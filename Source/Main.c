/* LICENSE
 *
 * Copyright © 2022 Blue-Maned_Hawk.  All rights reserved.
 *
 * This project should have come with a file called `LICENSE`.  In the event of any conflict between this comment and that file, that file shall be considered the authority.
 *
 * You may freely use this software.  You may freely distribute this software, so long as you distribute the license and source code with it.  You may freely modify this software and distribute the modifications under a similar license, so long as you distribute the sources with them and you don't claim that they're the original software.  None of this overrides local laws, and if you excercise these rights, you cannot claim that your actions are condoned by the author.
 *
 * This license does not apply to patents or trademarks.
 *
 * This software comes with no warranty, implied or explicit.  The author disclaims any liability for damages caused by this software. */

/* This is the main file for ëšho'hlorẓûţc hwomùaržrıtéu-erţtenļıls. */

#include <unistd.h>
#include <GLFW/glfw3.h>

int main([[maybe_unused]] int argc, [[maybe_unused]] const char ** argv){
	if (!glfwInit())
		return 1;
	GLFWwindow * window = glfwCreateWindow(640, 480, "ëšho'hlorẓûţc hwomùaržrıtéu-erţtenļıls", NULL, NULL);
	if (window == NULL)
		return 1;
	sleep(5);
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
